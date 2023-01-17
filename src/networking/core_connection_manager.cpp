/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/networking/core_connection_manager.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

// CoreConnectionManager constructor for Core Controller.
CoreConnectionManager::CoreConnectionManager (const std::string& controller_address) :
    m_control_application_ptr { nullptr },
    core_address { controller_address },
    index_t { 0 }
{ }

//    CoreConnectionManager default destructor.
CoreConnectionManager::~CoreConnectionManager () = default;

//    Start call. Continuously accept connections from Local Controllers.
void CoreConnectionManager::Start (ControlApplication* app_ptr)
{
    m_control_application_ptr = dynamic_cast<CoreControlApplication*> (app_ptr);

    ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort (core_address, grpc::InsecureServerCredentials ());

    // Register "service" as the instance through which we'll communicate with clients.
    // In this case it corresponds to an *synchronous* service.
    builder.RegisterService (this);

    // Finally assemble the server.
    server = builder.BuildAndStart ();
    std::cout << "Server listening on " << core_address << std::endl;

    // Wait for the server to shutdown.
    // Note that some other thread must be responsible for shutting down the server for this call to
    // ever return.
    server->Wait ();

    std::cout << "Waiting for connection ...\n";
}

// Connect local controller to core controller
Status CoreConnectionManager::ConnectLocalToGlobal (ServerContext* context,
    const ConnectRequest* request,
    ConnectReply* reply)
{
    std::string prefix ("Hello user with address: ");

    PStatus status;

    if (!request->user_address ().empty ()) {
        // register data plane session

        if (m_control_application_ptr != nullptr) {
            LocalControllerSession* ptr_t
                = m_control_application_ptr->register_local_controller_session (
                    request->user_address ());

            // spawn thread for the new data plane session
            // asio::post (thread_pool_, [&] () { ptr_t->StartSession (request->user_address()); });
            std::thread session_thread_t = std::thread (&LocalControllerSession::StartSession,
                ptr_t,
                request->user_address ());
            session_thread_t.detach ();

            // update index and connection delay
            index_t++;

            Logging::log_info (
                "Connecting (" + std::to_string (index_t) + ") and going for the next one ...");
        } else {
            reply->set_message ("Connection to Core Controller is not possible\n");
            return Status::CANCELLED;
        }
    }

    reply->set_message (prefix + request->user_address ());
    return Status::OK;
}

// Connect data plane stage to core controller
Status CoreConnectionManager::ConnectStageToGlobal (ServerContext* context,
    const StageInfo* request,
    ConnectReply* reply)
{
    std::string prefix ("Hello stage with index: ");

    PStatus status;

    if (!request->local_address ().empty ()) {
        // register data plane session
        if (m_control_application_ptr != nullptr) {
            m_control_application_ptr->register_stage_session (request->local_address (),
                request->stage_name (),
                request->stage_env (),
                request->stage_user ());

            Logging::log_info ("Connecting stage (" + request->stage_name () + ") from "
                + request->local_address () + " and going for the next one ...");
        } else {
            reply->set_message ("Connection to Core Controller is not possible\n");
            return Status::CANCELLED;
        }
    }

    reply->set_message (prefix + request->stage_name () + " " + request->stage_env ());
    return Status::OK;
}

void CoreConnectionManager::Stop ()
{
    server->Shutdown ();
    m_control_application_ptr->stop_feedback_loop ();
}

} // namespace cheferd
