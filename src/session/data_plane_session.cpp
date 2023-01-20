/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/session/data_plane_session.hpp>

namespace cheferd {

// DataPlaneSession default constructor.
DataPlaneSession::DataPlaneSession (const char* socket_name) : session_id_ { 0 }, interface_ {}
{
    PrepareUnixConnection (socket_name);
}

// DataPlaneSession parameterized constructor.
DataPlaneSession::DataPlaneSession (long id, const char* socket_name) :
    session_id_ { id },
    interface_ {}
{
    PrepareUnixConnection (socket_name);
}

// DataPlaneSession default destructor.
DataPlaneSession::~DataPlaneSession () = default;

void DataPlaneSession::PrepareUnixConnection (const char* socket_name)
{
    unlink (socket_name);

    if ((server_fd_ = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        Logging::log_error ("Socket creation error.");
        exit (EXIT_FAILURE);
    }

    //    setsockopt(server_fd_,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT|SO_NOSIGPIPE,&opt,sizeof(opt));

    unix_socket_.sun_family = AF_UNIX;
    strncpy (unix_socket_.sun_path, socket_name, sizeof (unix_socket_.sun_path) - 1);

    if (bind (server_fd_, (struct sockaddr*)&unix_socket_, sizeof (unix_socket_)) < 0) {
        Logging::log_error ("Bind error.");
        working_session_ = false;
    }

    if (listen (server_fd_, 3) < 0) {
        Logging::log_error ("Listen error.");
        working_session_ = false;
    }

    addrlen_ = sizeof (unix_socket_);
}

// StartSession() call. Start session between the controller and the data plane
// stage.
void DataPlaneSession::StartSession ()
{
    Logging::log_debug ("DataPlaneSession::StartSession");

    int socket_t = accept (server_fd_, (struct sockaddr*)&unix_socket_, (socklen_t*)&addrlen_);

    // verify socket value
    socket_t == -1 ? Logging::log_error ("DataPlaneSession: failed to connect with "
                                         "data plane stage {UNIX}.")
                   : Logging::log_debug ("New data plane stage connection established {UNIX}.");

    if (socket_t != -1) {
        working_session_ = true;
    }

    // after knowing the Stage identifier,
    while (working_session_.load ()) {

        PStatus status;
        ControlOperation operation {};
        std::string rule {};

        status = DequeueRuleFromSubmissionQueue (rule);

        if (status.isOk ()) {
            status = SendRule (socket_t, rule, &operation);
        }
    }

    Logging::log_debug ("DataPlaneSession :: Exiting data plane stage session.");
}

// SendRule call. Submit rule to the data plane stage.
PStatus
DataPlaneSession::SendRule (int socket, const std::string& rule, ControlOperation* operation)
{
    // parse rule
    if (!rule.empty ()) {
        std::string token = rule.substr (0, rule.find ('|'));
        // std::cout << "Token: " << token << "\n";
        operation->m_operation_type = std::stoi (token);
    }

    PStatus status = PStatus::Error ();
    switch (operation->m_operation_type) {
        case STAGE_HANDSHAKE: {
            operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);
            // create temporary StageHandshakeRAW structure
            StageSimplifiedHandshakeRaw handshake_obj {};
            // invoke SouthboundInterface's StageHandshake call
            status = interface_.stage_handshake (socket, operation, handshake_obj);
            // enqueue response of data plane stage from StageHandshake request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseHandshake> (STAGE_HANDSHAKE, handshake_obj));
            break;
        }

        case STAGE_READY: {
            std::cout << "STAGE_READY ...\n";
            operation->m_size = sizeof (struct StageReadyRaw);
            // create temporary StageReadyRaw struct
            StageReadyRaw stage_ready {};
            // create temporary ACK structure
            ACK ack {};
            // invoke ...
            status = interface_.mark_stage_ready (socket, operation, stage_ready, ack);
            // enqueue response of data plane stage from mar_stage_ready request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (STAGE_READY, ack.m_message));
            break;
        }

        case CREATE_ENF_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateEnforcementRule
            status = interface_.create_enforcement_rule (socket, operation, rule, ack);

            // enqueue response of data plane stage from CreateEnforcementRule
            // request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (CREATE_ENF_RULE, ack.m_message));

            break;
        }

        case REMOVE_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's RemoveRule
            status = interface_.RemoveRule (socket, operation, operation->m_operation_id, ack);
            // enqueue response of data plane stage from RemoveRule request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (REMOVE_RULE, ack.m_message));
            break;
        }

        case COLLECT_STATS:
            status = interface_.collect_statistics (socket, operation);
            break;

        case COLLECT_DETAILED_STATS: {
            std::vector<std::string> tokens {};

            size_t start;
            size_t end = 0;

            while ((start = rule.find_first_not_of ('|', end)) != std::string::npos) {
                end = rule.find ('|', start);
                tokens.push_back (rule.substr (start, end - start));
            }
            operation->m_operation_subtype = std::stoi (tokens[1]);

            switch (operation->m_operation_subtype) {
                case COLLECT_GLOBAL_STATS: {
                    // create temporary StatsGlobalRaw structure
                    StatsGlobalRaw stats_global {};
                    // invoke SouthboundInterface's CollectStatisticsKVS
                    status = interface_.collect_global_statistics (socket, operation, stats_global);

                    if (status.isOk ()) {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        // request
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStat> (COLLECT_GLOBAL_STATS,
                                stats_global.m_total_rate));
                    } else {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        // request
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStat> (COLLECT_GLOBAL_STATS, -1));
                    }
                    break;
                }

                default:
                    Logging::log_error ("After parsing -- other rule");
                    return PStatus::Error ();
            }
            break;
        }

        default:
            status = PStatus::NotSupported ();
            Logging::log_error ("PAI/O Interface:.SendRule -- rule not supported.");
            break;
    }

    //        std::cout << "Object removed ...\n";
    return status;
}

void DataPlaneSession::RemoveSession ()
{
    working_session_ = false;
    EnqueueRuleInSubmissionQueue ("");
}

// EnqueueRuleInSubmissionQueue call. Enqueue rule in the submission_queue.
void DataPlaneSession::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue");

    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();

    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue2");
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the
//  execution ... DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue.
PStatus DataPlaneSession::DequeueRuleFromSubmissionQueue (std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    PStatus status_t = PStatus::Error ();

    // Logging::log_debug("DataPlaneSession :: 1Dequeueue to dequeue");

    while (working_session_.load () && submission_queue_.empty ()) {
        // Logging::log_debug("DataPlaneSession :: 2Dequeueue to dequeue");

        submission_queue_condition_.wait (lock_t);
        // Logging::log_debug("DataPlaneSession :: 3Dequeueue to dequeue");
    }

    // Logging::log_debug("DataPlaneSession :: 4Dequeueue to dequeue");

    if (working_session_.load ()) {
        // Logging::log_debug("DataPlaneSession :: 5Dequeueue to dequeue");
        rule = submission_queue_.front ();
        submission_queue_.pop ();
        status_t = PStatus::OK ();
    }

    return status_t;
}

// Missing: probably some marshaling and unmarshaling needs to be done in this method
//  EnqueueResponseInCompletionQueue call. Enqueue response in the completion_queue.
void DataPlaneSession::EnqueueResponseInCompletionQueue (
    std::unique_ptr<StageResponse> response_object)
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };
    completion_queue_.emplace (std::move (response_object));
    completion_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the execution
//  DequeueResponseFromCompletionQueue call. Dequeue response from the completion_queue.
std::unique_ptr<StageResponse> DataPlaneSession::DequeueResponseFromCompletionQueue ()
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };

    while (completion_queue_.empty ()) {

        completion_queue_condition_.wait (lock_t);
    }

    std::unique_ptr<StageResponse> response_t = std::move (completion_queue_.front ());
    completion_queue_.pop ();

    return response_t;
}

//    GetSubmissionQueueSize call. Return the size of the submission_queue.
int DataPlaneSession::getSubmissionQueueSize ()
{
    return submission_queue_.size ();
}

//    SubmitRule call. Submit rules to the Session.
PStatus DataPlaneSession::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

//    GetResult call. Get results from the Session.
std::unique_ptr<StageResponse> DataPlaneSession::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

long DataPlaneSession::SessionIdentifier () const
{
    return session_id_;
}

} // namespace cheferd
