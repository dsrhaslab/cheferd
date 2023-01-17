/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/session/session.hpp>

namespace cheferd {

// Session default constructor.
Session::Session () :
    session_id_ { 0 },
    submission_queue_ {},
    submission_queue_condition_ {},
    completion_queue_ {}
{ }

// Session parameterized constructor.
Session::Session (long id) :
    session_id_ { id },
    submission_queue_ {},
    submission_queue_condition_ {},
    completion_queue_ {}
{ }

// Session default destructor.
Session::~Session () = default;

// EnqueueRuleInSubmissionQueue call. Enqueue rule in the submission_queue.
void Session::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue");

    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();

    // Logging::log_debug("DataPlaneSession :: Enqueueue to dequeue2");
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the
//  execution ... DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue.
PStatus Session::DequeueRuleFromSubmissionQueue (std::string& rule)
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
void Session::EnqueueResponseInCompletionQueue (std::unique_ptr<StageResponse> response_object)
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };
    completion_queue_.emplace (std::move (response_object));
    completion_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the execution
//  DequeueResponseFromCompletionQueue call. Dequeue response from the completion_queue.
std::unique_ptr<StageResponse> Session::DequeueResponseFromCompletionQueue ()
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
int Session::getSubmissionQueueSize ()
{
    return submission_queue_.size ();
}

//    SubmitRule call. Submit rules to the Session.
PStatus Session::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();
    Logging::log_debug ("Session: SubmitRule");

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

//    GetResult call. Get results from the Session.
std::unique_ptr<StageResponse> Session::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

long Session::SessionIdentifier () const
{
    return session_id_;
}

} // namespace cheferd
