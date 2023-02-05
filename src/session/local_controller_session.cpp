/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_stat.hpp"

#include <cheferd/session/local_controller_session.hpp>

namespace cheferd {

// LocalControllerSession parameterized constructor.
LocalControllerSession::LocalControllerSession (const std::string& user_address) :
    session_id_ { 0 },
    interface_ { user_address }
{ }

// LocalControllerSession parameterized constructor.
LocalControllerSession::LocalControllerSession (long id, const std::string& user_address) :
    session_id_ { id },
    interface_ { user_address }
{ }

// LocalControllerSession default destructor.
LocalControllerSession::~LocalControllerSession () = default;

// StartSession call. Start session execution.
void LocalControllerSession::StartSession (const std::string& user_address)
{
    Logging::log_debug ("LocalControllerSession::StartSession");

    working_session_ = true;

    // after knowing the Stage identifier,
    while (working_session_.load ()) {
        PStatus status;
        ControlOperation operation {};
        std::string rule {};

        status = DequeueRuleFromSubmissionQueue (rule);
        if (status.isOk ()) {
            status = SendRule (user_address, rule, &operation);
        }
    }
}

// SendRule call. Handle the rule to be submitted to the local controller.
PStatus LocalControllerSession::SendRule (const std::string& user_address,
    const std::string& rule,
    ControlOperation* operation)
{
    // parse rule
    if (!rule.empty ()) {
        std::string token = rule.substr (0, rule.find ('|'));
        operation->m_operation_type = std::stoi (token);
    }

    PStatus status = PStatus::Error ();
    switch (operation->m_operation_type) {
        case LOCAL_HANDSHAKE: {
            operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's StageHandshake call
            status = interface_.local_handshake (user_address, operation, rule, ack);
            // enqueue response of data plane stage from StageHandshake request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (LOCAL_HANDSHAKE, ack.m_message));

            break;
        }

        case STAGE_HANDSHAKE: {
            operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);
            // create temporary StageHandshakeRAW structure
            StageSimplifiedHandshakeRaw handshake_obj {};
            // invoke SouthboundInterface's StageHandshake call
            status = interface_.stage_handshake (user_address, operation, handshake_obj);
            // enqueue response of data plane stage from StageHandshake request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseHandshake> (STAGE_HANDSHAKE, handshake_obj));

            break;
        }

        case STAGE_READY: {
            operation->m_size = sizeof (struct StageReadyRaw);
            // create temporary ACK structure
            ACK ack {};
            // invoke ...
            status = interface_.mark_stage_ready (user_address, operation, rule, ack);
            // enqueue response of data plane stage from mar_stage_ready request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (STAGE_READY, ack.m_message));

            break;
        }

        case CREATE_ENF_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateEnforcementRule
            status = interface_.create_enforcement_rule (user_address, operation, rule, ack);

            // enqueue response of data plane stage from CreateEnforcementRule
            // request
            EnqueueResponseInCompletionQueue (
                std::make_unique<StageResponseACK> (CREATE_ENF_RULE, ack.m_message));

            break;
        }

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
                    // create temporary StatsKVSRaw structure
                    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>
                        stats_tf_objects = std::make_unique<
                            std::unordered_map<std::string, std::unique_ptr<StageResponse>>> ();

                    // invoke SouthboundInterface's CollectStatisticsKVS
                    status = interface_.collect_global_statistics (user_address,
                        operation,
                        stats_tf_objects);

                    if (status.isOk ()) {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS,
                                stats_tf_objects));

                    } else {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS,
                                stats_tf_objects));
                    }
                    break;
                }
                case COLLECT_GLOBAL_STATS_AGGREGATED: {
                    // create temporary StatsKVSRaw structure
                    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>
                        stats_tf_objects = std::make_unique<
                            std::unordered_map<std::string, std::unique_ptr<StageResponse>>> ();

                    // invoke SouthboundInterface's CollectStatisticsKVS
                    status = interface_.collect_global_statistics_aggregated (user_address,
                        operation,
                        stats_tf_objects);

                    if (status.isOk ()) {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS_AGGREGATED,
                                stats_tf_objects));

                    } else {
                        // enqueue response of data plane stage from collect_tensorflow_statistics
                        EnqueueResponseInCompletionQueue (
                            std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS_AGGREGATED,
                                stats_tf_objects));
                    }
                    break;
                }

                default:
                    Logging::log_error ("LocalControllerSession: After parsing -- other rule");
                    return PStatus::Error ();
            }
            break;
        }
        default:
            status = PStatus::NotSupported ();
            Logging::log_error ("LocalControllerSession: SendRule -- rule not supported.");
            break;
    }

    return status;
}

// RemoveSession call. Stop session execution.
void LocalControllerSession::RemoveSession ()
{
    working_session_ = false;
    EnqueueRuleInSubmissionQueue ("");
}

// EnqueueRuleInSubmissionQueue call. Enqueue rule in the submission_queue_ in
// string-based format.
void LocalControllerSession::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();
}

// DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue_
// in string-based format.
PStatus LocalControllerSession::DequeueRuleFromSubmissionQueue (std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    PStatus status_t = PStatus::Error ();

    while (working_session_.load () && submission_queue_.empty ()) {
        submission_queue_condition_.wait (lock_t);
    }

    if (working_session_.load ()) {
        rule = submission_queue_.front ();
        submission_queue_.pop ();
        status_t = PStatus::OK ();
    }

    return status_t;
}

// EnqueueResponseInCompletionQueue call. Enqueue response in the completion_queue_
// in StageResponse format.
void LocalControllerSession::EnqueueResponseInCompletionQueue (
    std::unique_ptr<StageResponse> response_object)
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };
    completion_queue_.emplace (std::move (response_object));
    completion_queue_condition_.notify_one ();
}

// DequeueResponseFromCompletionQueue call. Dequeue response from the
// completion_queue_ in StageResponse format.
std::unique_ptr<StageResponse> LocalControllerSession::DequeueResponseFromCompletionQueue ()
{

    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };

    while (completion_queue_.empty ()) {
        completion_queue_condition_.wait (lock_t);
    }

    std::unique_ptr<StageResponse> response_t = std::move (completion_queue_.front ());
    completion_queue_.pop ();

    return response_t;
}

// getSubmissionQueueSize call. Get the total size of the submission_queue.
int LocalControllerSession::getSubmissionQueueSize ()
{
    return submission_queue_.size ();
}

// SubmitRule call. Submit rules to the Session.
PStatus LocalControllerSession::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

// GetResult call. Pop result objects (StageResponse) from the Session.
std::unique_ptr<StageResponse> LocalControllerSession::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

// SessionIdentifier call. Get session identifier.
long LocalControllerSession::SessionIdentifier () const
{
    return session_id_;
}

} // namespace cheferd
