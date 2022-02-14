/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/networking/connection_manager.hpp>
#include <shepherd/utils/logging.hpp>

namespace shepherd {


// ConnectionManager constructor for Core Controller.
ConnectionManager::ConnectionManager (const std::string& controller_address) :
    server_fd_ { -1 },
    addrlen_ { -1 },
    server_type_ { option_communication_ },
    thread_pool_ { option_max_connections_ },
    m_control_application_ptr {nullptr},
    core_address { controller_address },
    index_t { 0 }
{}

// ConnectionManager constructor for Local Controller.
ConnectionManager::ConnectionManager (const std::string& controller_address,  const std::string& local_address) :
    server_fd_ { -1 },
    addrlen_ { -1 },
    unix_socket_array_ {},
    server_fd_array_ {},
    addrlen_array_ {},
    server_type_ { option_communication_ },
    thread_pool_ { option_max_connections_ },
    index_t { 0 }
{

    int prepare_value_t;
    switch (server_type_) {
        // create a UNIX Domain Sockets connection
        case CommunicationType::UNIX: {

            std::string control_type = (option_is_rocksdb_algorithm_ ? "kvs" : "tensorflow");
            if (control_type.compare ("tensorflow") == 0) {
                PrepareUnixConnections (option_socket_name_tf_1_.c_str (), 0);
                PrepareUnixConnections (option_socket_name_tf_2_.c_str (), 1);
                PrepareUnixConnections (option_socket_name_tf_3_.c_str (), 2);
                PrepareUnixConnections (option_socket_name_tf_4_.c_str (), 3);
            } else {
                Logging::log_debug ("PrepareUnixConnection: connecting RocksDB "
                                    "instance through UNIX sockets.");
                PrepareUnixConnection (option_socket_name_.c_str (), option_backlog_);
            }
            break;
        }
            // prepare for INET socket communications
        case CommunicationType::INET: {
            prepare_value_t = PrepareInetConnection (option_port_);

            if (prepare_value_t == -1) {
                Logging::log_error ("ConnectionManager: failed to prepare INET Socket connection.");
            }
            break;

        }
        // unknown communication type
        default:
            Logging::log_error ("Communication type not supported.");
            break;
    }

}


//    ConnectionManager default destructor.
ConnectionManager::~ConnectionManager () = default;


////////////// Core Controller ///////////////

//    Start call. Continuously accept connections from Local Controllers.
void ConnectionManager::Start (CoreControlApplication* app_ptr)
{
    m_control_application_ptr = app_ptr;

    ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(core_address, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with clients.
    // In this case it corresponds to an *synchronous* service.
    builder.RegisterService(this);

    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << core_address << std::endl;

    // Wait for the server to shutdown.
    // Note that some other thread must be responsible for shutting down the server for this call to ever return.
    server->Wait();

    std::cout << "Waiting for connection ...\n";
}

// Connect local controller to core controller
Status ConnectionManager::ConnectLocalToGlobal(ServerContext* context, const ConnectRequest* request,
    ConnectReply* reply)  {
    std::string prefix("Hello user with address: ");

    PStatus status;

    if (!request->user_address().empty()) {
        // register data plane session

        if (m_control_application_ptr != nullptr){
            LocalControllerSession* ptr_t = m_control_application_ptr->register_local_controller_session (request->user_address(), index_t);

            // spawn thread for the new data plane session
            asio::post (thread_pool_, [&] () { ptr_t->StartSession (request->user_address()); });

            // update index and connection delay
            index_t++;

            Logging::log_info (
                "Connecting (" + std::to_string (index_t) + ") and going for the next one ...");
        }
        else {
            reply->set_message("Connection to Core Controller is not possible\n");
            return Status::CANCELLED;
        }


    }

    reply->set_message(prefix + request->user_address());
    return Status::OK;
}

// Connect data plane stage to core controller
Status ConnectionManager::ConnectStageToGlobal(ServerContext* context, const ConnectRequestStage* request,
    ConnectReply* reply)  {
    std::string prefix("Hello stage with index: ");

    PStatus status;

    if (!request->local_address().empty()) {
        // register data plane session
        if (m_control_application_ptr != nullptr){
            m_control_application_ptr->register_stage_session (request->local_address(), request->stage_name(), request->stage_env(), request->stage_user());

            Logging::log_info (
                "Connecting stage (" + request->stage_name() + ") from " + request->local_address() + " and going for the next one ...");
        }
        else {
            reply->set_message("Connection to Core Controller is not possible\n");
            return Status::CANCELLED;
        }
    }

    reply->set_message(prefix + request->stage_name() + " " + request->stage_env());
    return Status::OK;
}

/////////////////


////////////// Local Controller ///////////////

//    Start call. Continuously accept connections from Local Controllers.
void ConnectionManager::Start (LocalControlApplication* app_ptr)
{
    index_t = 0;
    int delay_t = INITIAL_CONNECTION_DELAY;

    while (true) {
        std::cout << "Waiting for connection ...\n";

        // accept data plane stage connection (TensorFlow)
        int socket_t = AcceptConnections (index_t);

        if (socket_t != -1) {
            // register data plane session
            DataPlaneSession* ptr_t = app_ptr->register_stage_session (index_t);
            std::cout << "Pointer: " << ptr_t << "\n";

            // spawn thread for the new data plane session
            asio::post (thread_pool_, [&] () { ptr_t->StartSession (socket_t); });

            // update index and connection delay
            index_t++;
            delay_t = INITIAL_CONNECTION_DELAY;

            Logging::log_info (
                "Connecting (" + std::to_string (index_t) + ") and going for the next one ...");
        } else {
            // exponential backoff based algorithm to delay the connection of a
            // new data plane stage
            std::this_thread::sleep_for (microseconds (delay_t));
            if (delay_t < MAX_CONNECTION_DELAY) {
                delay_t *= 2;
            }
        }
    }
}

//    PrepareInetConnection call. Prepare INET-based connections between the
//    control plane and the data plane stage.
int ConnectionManager::PrepareInetConnection (int port)
{
    Logging::log_info ("ConnectionManager: establishing INET connection with "
                       "data plane stage.");

    // Creating socket file descriptor
    if ((server_fd_ = socket (AF_INET, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("Socket creation error.");
        exit (EXIT_FAILURE);
    }

    inet_socket_.sin_family = AF_INET;
    inet_socket_.sin_addr.s_addr = INADDR_ANY;
    inet_socket_.sin_port = htons (port);

    if (bind (server_fd_, (struct sockaddr*)&inet_socket_, sizeof (inet_socket_)) < 0) {
        Logging::log_error ("Bind error.");
        return -1;
    }

    if (listen (server_fd_, 3) < 0) {
        Logging::log_error ("Listen error.");
        return -1;
    }

    return 0;
}

//    PrepareUnixConnection call. Prepare UNIX Domain socket connections between
//    the control plane and the data plane stage.
int ConnectionManager::PrepareUnixConnection (const char* socket_name, int backlog)
{
    Logging::log_info ("ConnectionManager: establishing UNIX connection with "
                       "data plane stage.");

    // creating socket file descriptor
    unlink (socket_name);

    if ((server_fd_ = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("Socket creation error.");
        exit (EXIT_FAILURE);
    }

    unix_socket_.sun_family = AF_UNIX;
    strncpy (unix_socket_.sun_path, socket_name, sizeof (unix_socket_.sun_path) - 1);

    if (bind (server_fd_, (struct sockaddr*)&unix_socket_, sizeof (unix_socket_)) < 0) {
        Logging::log_error ("Bind error.");
        return -1;
    }

    if (listen (server_fd_, 3) < 0) {
        Logging::log_error ("Listen error.");
        return -1;
    }

    addrlen_ = sizeof (unix_socket_);

    return 0;
}

int ConnectionManager::PrepareUnixConnections (const char* socket_name, int index)
{
    Logging::log_info ("UnixConnection: connecting instance-"
        + std::to_string (index + 1) + " through UNIX sockets.");

    // creating socket file descriptor
    unlink (socket_name);

    if ((server_fd_array_[index] = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("Socket creation error.");
        exit (EXIT_FAILURE);
    }

    unix_socket_array_[index].sun_family = AF_UNIX;
    strncpy (unix_socket_array_[index].sun_path,
        socket_name,
        sizeof (unix_socket_array_[index].sun_path) - 1);

    if (bind (server_fd_array_[index],
            (struct sockaddr*)&unix_socket_array_[index],
            sizeof (unix_socket_array_[index]))
        < 0) {
        Logging::log_error ("Bind error.");
        return -1;
    }

    if (listen (server_fd_array_[index], 3) < 0) {
        Logging::log_error ("Listen error.");
        return -1;
    }

    addrlen_array_[index] = sizeof (unix_socket_array_[index]);

    Logging::log_debug ("Socket [" + std::to_string (index) + "] = "
        + std::to_string (server_fd_array_[index]) + ", " + std::to_string (addrlen_array_[index]));

    return 0;
}


//    Accept call. establish a new connection between the control plane and a
//    data plane stage.
int ConnectionManager::Accept ()
{
    int new_socket_t = -1;

    switch (server_type_) {
        case CommunicationType::UNIX:
            // accept new data plane stage connection
            new_socket_t
                = accept (server_fd_, (struct sockaddr*)&unix_socket_, (socklen_t*)&addrlen_);

            // verify socket value
            new_socket_t == -1
                ? Logging::log_error ("ConnectionManager: failed to connect with "
                                      "data plane stage {UNIX}.")
                : Logging::log_debug ("New data plane stage connection established {UNIX}.");

            break;

        case CommunicationType::INET:
            // accept new data plane stage connection
            new_socket_t
                = accept (server_fd_, (struct sockaddr*)&inet_socket_, (socklen_t*)&addrlen_);

            // verify socket value
            new_socket_t == -1
                ? Logging::log_error ("ConnectionManager: failed to connect with "
                                      "data plane stage {INET}.")
                : Logging::log_debug ("New data plane stage connection established {INET}.");

            break;

        default:
            Logging::log_error ("Communication type not supported.");
            break;
    }

    return new_socket_t;
}

//    AcceptConnections call. Establish a new connection between the
//    control plane and a data plane stage.
int ConnectionManager::AcceptConnections (int index)
{
    int new_socket_t = -1;

    Logging::log_info ("AcceptConnections:: " + std::to_string (index));
    switch (server_type_) {
        case CommunicationType::UNIX:
            // accept new data plane stage connection
            new_socket_t = accept (server_fd_array_[index],
                (struct sockaddr*)&unix_socket_array_[index],
                (socklen_t*)&addrlen_array_[index]);

            // verify socket value
            new_socket_t == -1 ? Logging::log_error ("ConnectionManager: failed to connect with "
                                                     "data plane stage {UNIX}.")
                               : Logging::log_info ("New data plane stage connection established "
                                                    "{UNIX} (tensorflow-"
                                   + std::to_string (index) + ").");
            break;

        case CommunicationType::gRPC:

            break;
        default:
            Logging::log_error ("Communication type not supported.");
            break;
    }

    return new_socket_t;
}

///////////////////////

//    Operator call. Launch an ephemeral Data Plane Session.
void ConnectionManager::operator() (int socket, DataPlaneSession* session)
{
    if (session == nullptr) {
        Logging::log_error ("ConnectionManager operator -- nullptr session.");
    } else {
        std::cout << "Session address: " << &session << "\n";
    }
    session->StartSession (socket);
}

} // namespace shepherd
