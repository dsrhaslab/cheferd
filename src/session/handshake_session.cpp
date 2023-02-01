/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/session/handshake_session.hpp>

namespace cheferd {

// HandshakeSession default constructor.
HandshakeSession::HandshakeSession () : socket_id_ { 0 }, interface_ {}
{ }

// HandshakeSession parameterized constructor.
HandshakeSession::HandshakeSession (long id) : socket_id_ { id }, interface_ {}
{ }

// HandshakeSession default destructor.
HandshakeSession::~HandshakeSession () = default;

// StartSession() call. Start session between the controller and the data plane
// stage.
void HandshakeSession::StartSession ()
{

    Logging::log_debug ("HandshakeSession :: " + std::to_string (getSubmissionQueueSize ()));

    PStatus status;
    ControlOperation operation {};
    std::string rule {};

    working_session_ = true;

    status = DequeueRuleFromSubmissionQueue (rule);

    if (status.isOk ()) {
        status = SendRule (socket_id_, rule, &operation);
    }

    if (working_session_.load () && status.isOk ()) {
        /* Send info about address and port to connect to */
        status = DequeueRuleFromSubmissionQueue (rule);
        if (status.isOk ()) {
            status = SendRule (socket_id_, rule, &operation);
        }
    }
}

// SendRule call. Submit rule to the data plane stage.
PStatus
HandshakeSession::SendRule (int socket, const std::string& rule, ControlOperation* operation)
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

        case STAGE_HANDSHAKE_INFO: {
            // create temporary ACK structure
            ACK ack {};

            // send to the data plane stage the address and port that it should connect to.
            status = interface_.stage_handshake_address (socket, rule, ack);

            // enqueue response of data plane stage from StageHandshakeInfo
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (STAGE_HANDSHAKE_INFO, ack.m_message));
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

void HandshakeSession::RemoveSession ()
{
    working_session_ = false;
    EnqueueRuleInSubmissionQueue ("");
}

// EnqueueRuleInSubmissionQueue call. Enqueue rule in the submission_queue.
void HandshakeSession::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue");

    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();

    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue2");
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the
//  execution ... DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue.
PStatus HandshakeSession::DequeueRuleFromSubmissionQueue (std::string& rule)
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
void HandshakeSession::EnqueueResponseInCompletionQueue (
    std::unique_ptr<StageResponse> response_object)
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };
    completion_queue_.emplace (std::move (response_object));
    completion_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the execution
//  DequeueResponseFromCompletionQueue call. Dequeue response from the completion_queue.
std::unique_ptr<StageResponse> HandshakeSession::DequeueResponseFromCompletionQueue ()
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
int HandshakeSession::getSubmissionQueueSize ()
{
    return submission_queue_.size ();
}

//    SubmitRule call. Submit rules to the Session.
PStatus HandshakeSession::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

//    GetResult call. Get results from the Session.
std::unique_ptr<StageResponse> HandshakeSession::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

long HandshakeSession::SessionIdentifier () const
{
    return socket_id_;
}

} // namespace cheferd
