/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/networking/local_connection_manager.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

// LocalConnectionManager parameterized constructor.
LocalConnectionManager::LocalConnectionManager (const std::string& controller_address,
    const std::string& local_address) :
    server_fd_ { -1 },
    addrlen_ { -1 },
    unix_socket_array_ {},
    server_fd_array_ {},
    addrlen_array_ {},
    server_type_ { option_communication_ },
    index_t { 0 },
    working_connection_ { true }
{

    int prepare_value_t;
    switch (server_type_) {
        // create a UNIX Domain Sockets connection
        case CommunicationType::UNIX: {

            std::string socket_name = "/tmp/" + local_address + ".socket";
            PrepareUnixConnections (socket_name.c_str (), 0);

            break;
        }
        case CommunicationType::INET: {
            prepare_value_t = PrepareInetConnection (option_port_);

            if (prepare_value_t == -1) {
                Logging::log_error (
                    "LocalConnectionManager: failed to prepare INET Socket connection.");
            }
            break;
        }
        // unknown communication type
        default:
            Logging::log_error ("Communication type not supported.");
            break;
    }
}

// LocalConnectionManager default destructor.
LocalConnectionManager::~LocalConnectionManager () = default;

// Start call. Execute an endless loop that continuously accepts connections.
void LocalConnectionManager::Start (ControlApplication* app_ptr)
{
    index_t = 0;
    int delay_t = INITIAL_CONNECTION_DELAY;

    LocalControlApplication* ctr_ptr = dynamic_cast<LocalControlApplication*> (app_ptr);

    while (working_connection_.load ()) {
        Logging::log_info ("LocalConnectionManager: Waiting for connection... ");

        // accept data plane stage connection
        int socket_t = AcceptConnections (0);

        if (socket_t != -1) {
            // register data plane session
            ctr_ptr->register_stage_session (socket_t);

            // update index and connection delay
            index_t++;
            delay_t = INITIAL_CONNECTION_DELAY;

            Logging::log_info ("LocalConnectionManager: Connecting (" + std::to_string (index_t)
                + ") and going for the next one ...");
        } else {
            // exponential backoff based algorithm to delay the connection of a
            // new data plane stage
            std::this_thread::sleep_for (microseconds (delay_t));
            if (delay_t < MAX_CONNECTION_DELAY) {
                delay_t *= 2;
            }
        }
    }

    // TO-DO: Uniformizar a control application
    Logging::log_info ("LocalConnectionManager: Exiting Local Manager");
    ctr_ptr->stop_feedback_loop ();
}

// Stop call. Stop connection manager.
void LocalConnectionManager::Stop ()
{
    close (server_fd_array_[0]);
    working_connection_ = false;
}

// PrepareInetConnection call. Prepare INET-based connections between the
// control plane and the data plane stage.
int LocalConnectionManager::PrepareInetConnection (int port)
{
    Logging::log_info ("LocalConnectionManager: establishing INET connection with "
                       "data plane stage.");

    // Creating socket file descriptor
    if ((server_fd_ = socket (AF_INET, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("LocalConnectionManager: Socket creation error.");
        exit (EXIT_FAILURE);
    }

    inet_socket_.sin_family = AF_INET;
    inet_socket_.sin_addr.s_addr = INADDR_ANY;
    inet_socket_.sin_port = htons (port);

    if (bind (server_fd_, (struct sockaddr*)&inet_socket_, sizeof (inet_socket_)) < 0) {
        Logging::log_error ("LocalConnectionManager: Bind error.");
        return -1;
    }

    if (listen (server_fd_, 3) < 0) {
        Logging::log_error ("LocalConnectionManager: Listen error.");
        return -1;
    }

    return 0;
}

// PrepareUnixConnection call. Prepare UNIX Domain socket connections between
// the control plane and the data plane stage.
int LocalConnectionManager::PrepareUnixConnection (const char* socket_name, int backlog)
{
    Logging::log_info ("LocalConnectionManager: establishing UNIX connection with "
                       "data plane stage.");

    // creating socket file descriptor
    unlink (socket_name);

    if ((server_fd_ = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("LocalConnectionManager: Socket creation error.");
        exit (EXIT_FAILURE);
    }

    unix_socket_.sun_family = AF_UNIX;
    strncpy (unix_socket_.sun_path, socket_name, sizeof (unix_socket_.sun_path) - 1);

    if (bind (server_fd_, (struct sockaddr*)&unix_socket_, sizeof (unix_socket_)) < 0) {
        Logging::log_error ("LocalConnectionManager: Bind error.");
        return -1;
    }

    if (listen (server_fd_, 3) < 0) {
        Logging::log_error ("LocalConnectionManager: Listen error.");
        return -1;
    }

    addrlen_ = sizeof (unix_socket_);

    return 0;
}

// PrepareUnixConnections call. Prepare UNIX Domain socket connections
// between the control plane and the data plane stage (multiple).
int LocalConnectionManager::PrepareUnixConnections (const char* socket_name, int index)
{
    Logging::log_info ("LocalConnectionManager: UnixConnection: connecting instance-"
        + std::to_string (index + 1) + " through UNIX sockets.");

    // creating socket file descriptor
    unlink (socket_name);

    if ((server_fd_array_[index] = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("LocalConnectionManager: Socket creation error.");
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
        Logging::log_error ("LocalConnectionManager: Bind error.");
        return -1;
    }

    if (listen (server_fd_array_[index], 3) < 0) {
        Logging::log_error ("LocalConnectionManager: Listen error.");
        return -1;
    }

    addrlen_array_[index] = sizeof (unix_socket_array_[index]);

    Logging::log_debug ("LocalConnectionManager: Socket [" + std::to_string (index) + "] = "
        + std::to_string (server_fd_array_[index]) + ", " + std::to_string (addrlen_array_[index]));

    return 0;
}

// Accept call. Establish a new connection between the control plane and a data
// plane stage (single).
int LocalConnectionManager::Accept ()
{
    int new_socket_t = -1;

    switch (server_type_) {
        case CommunicationType::UNIX:
            // accept new data plane stage connection
            new_socket_t
                = accept (server_fd_, (struct sockaddr*)&unix_socket_, (socklen_t*)&addrlen_);

            // verify socket value
            new_socket_t == -1
                ? Logging::log_error ("LocalConnectionManager: failed to connect with "
                                      "data plane stage {UNIX}.")
                : Logging::log_debug ("LocalConnectionManager: New data plane stage "
                                      "connection established {UNIX}.");

            break;

        case CommunicationType::INET:
            // accept new data plane stage connection
            new_socket_t
                = accept (server_fd_, (struct sockaddr*)&inet_socket_, (socklen_t*)&addrlen_);

            // verify socket value
            new_socket_t == -1
                ? Logging::log_error ("LocalConnectionManager: failed to connect with "
                                      "data plane stage {INET}.")
                : Logging::log_debug ("LocalConnectionManager: New data plane stage "
                                      "connection established {INET}.");

            break;

        default:
            Logging::log_error ("LocalConnectionManager: Communication type not supported.");
            break;
    }

    return new_socket_t;
}

// AcceptConnections call. Establish a new connection between the control plane and a
// data plane stage (multiple).
int LocalConnectionManager::AcceptConnections (int index)
{
    int new_socket_t = -1;

    Logging::log_info ("LocalConnectionManager: AcceptConnections:: " + std::to_string (index));
    switch (server_type_) {
        case CommunicationType::UNIX:
            // accept new data plane stage connection
            new_socket_t = accept (server_fd_array_[index],
                (struct sockaddr*)&unix_socket_array_[index],
                (socklen_t*)&addrlen_array_[index]);

            break;

        case CommunicationType::gRPC:

            break;
        default:
            Logging::log_error ("LocalConnectionManager: Communication type not supported.");
            break;
    }

    return new_socket_t;
}

///////////////////////

// Operator call. Launch an Data Plane Session.
void LocalConnectionManager::operator() (int socket, DataPlaneSession* session)
{
    if (session == nullptr) {
        Logging::log_error ("LocalConnectionManager operator -- nullptr session.");
    }
    session->StartSession ();
}

} // namespace cheferd
