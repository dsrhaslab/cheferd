/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/session/local_controller_session.hpp>
#include "shepherd/networking/stage_response/stage_response_stats_global.hpp"

namespace shepherd {

// LocalControllerSession default constructor.
LocalControllerSession::LocalControllerSession (const std::string& user_address) :
    session_id_ { 0 },
    interface_ {user_address},
    submission_queue_ {},
    completion_queue_ {}
{ }

// LocalControllerSession parameterized constructor.
LocalControllerSession::LocalControllerSession (long id, const std::string& user_address) :
    session_id_ { id },
    interface_ {user_address},
    submission_queue_ {},
    completion_queue_ {}
{ }

// LocalControllerSession default destructor.
LocalControllerSession::~LocalControllerSession () = default;

// StartSession() call. Start session between the controller and the data plane
// stage.
void LocalControllerSession::StartSession (const std::string& user_address)
{
    Logging::log_debug ("LocalControllerSession::StartSession");

    Logging::log_debug ("LocalControllerSession :: " + std::to_string (getSubmissionQueueSize ()));

    int i = 1;

    // after knowing the Stage identifier,
    while (i) {
        PStatus status;
        ControlOperation operation {};
        std::string rule {};

        status = DequeueRuleFromSubmissionQueue (rule);

        status = SendRule (user_address, rule, &operation);
    }
}

// SendRule call. Submit rule to the data plane stage.
PStatus LocalControllerSession::SendRule (const std::string& user_address, const std::string& rule, ControlOperation* operation)
{
    // parse rule
    if (!rule.empty ()) {
        std::string token = rule.substr (0, rule.find ('|'));
        // std::cout << "Token: " << token << "\n";
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
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (LOCAL_HANDSHAKE, ack.m_message));

            break;
        }

        case STAGE_HANDSHAKE: {
            operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);
            // create temporary StageHandshakeRAW structure
            StageSimplifiedHandshakeRaw handshake_obj {};
            // invoke SouthboundInterface's StageHandshake call
            status = interface_.stage_handshake (user_address, operation, handshake_obj);
            // enqueue response of data plane stage from StageHandshake request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseHandshake> (STAGE_HANDSHAKE, handshake_obj));

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
            status = interface_.mark_stage_ready (user_address, operation, stage_ready, ack);
            // enqueue response of data plane stage from mar_stage_ready request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (STAGE_READY, ack.m_message));
            break;
        }

        case CREATE_HSK_RULE: {
            std::cout << "CREATE_HSK_RULE ... \n";
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateHousekeepingRule
            status = interface_.create_housekeeping_rule (user_address, operation, rule, ack);
            // enqueue response of data plane stage from CreateHousekeepingRule request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (CREATE_HSK_RULE, ack.m_message));
            break;
        }

        case EXEC_HSK_RULES: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's ExecuteHousekeepingRule
            status = interface_.ExecuteHousekeepingRules (user_address, operation, rule, ack);
            // enqueue response of data plane stage from ExecuteHousekeepingRule
            // request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (EXEC_HSK_RULES, ack.m_message));
            break;
        }

        case CREATE_ENF_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateEnforcementRule
            status = interface_.create_enforcement_rule (user_address, operation, rule, ack);

            // enqueue response of data plane stage from CreateEnforcementRule
            // request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (CREATE_ENF_RULE, ack.m_message));

            break;
        }

        case REMOVE_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's RemoveRule
            //status = interface_.RemoveRule (socket, operation, operation->m_operation_id, ack);
            // enqueue response of data plane stage from RemoveRule request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (REMOVE_RULE, ack.m_message));
            break;
        }

        case COLLECT_STATS:
            status = interface_.collect_statistics (user_address, operation);
            break;

        case COLLECT_GLOBAL_STATS: {
            // create temporary StatsKVSRaw structure
            std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>> stats_tf_objects =
                std::make_unique<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>();

            // invoke SouthboundInterface's CollectStatisticsKVS
            status = interface_.collect_global_statistics(user_address, operation, stats_tf_objects);

            if (status.isOk ()) {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS, stats_tf_objects
                        ));

            } else {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStats> (COLLECT_GLOBAL_STATS, stats_tf_objects));
            }
            break;
        }

        case COLLECT_ENTITY_STATS: {

            // create temporary StatsKVSRaw structure
            std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>> stats_tf_objects =
                std::make_unique<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>();

            // invoke SouthboundInterface's CollectStatisticsKVS
            status = interface_.collect_entity_statistics(user_address, operation, stats_tf_objects);

            if (status.isOk ()) {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStats> (COLLECT_ENTITY_STATS, stats_tf_objects
                        ));

            } else {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStats> (COLLECT_ENTITY_STATS, stats_tf_objects));
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

// EnqueueRuleInSubmissionQueue call. Enqueue rule in the submission_queue.
void LocalControllerSession::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the
//  execution ... DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue.
PStatus LocalControllerSession::DequeueRuleFromSubmissionQueue (std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    PStatus status_t = PStatus::Error ();

    while (submission_queue_.empty ()) {
        submission_queue_condition_.wait (lock_t);
    }

    rule = submission_queue_.front ();
    submission_queue_.pop ();

    status_t = PStatus::OK ();

    return status_t;
}

// Missing: probably some marshaling and unmarshaling needs to be done in this method
//  EnqueueResponseInCompletionQueue call. Enqueue response in the completion_queue.
void LocalControllerSession::EnqueueResponseInCompletionQueue (
    std::unique_ptr<StageResponse> response_object)
{
    std::unique_lock<std::mutex> lock_t { completion_queue_lock_ };
    completion_queue_.emplace (std::move (response_object));
    completion_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the execution
//  DequeueResponseFromCompletionQueue call. Dequeue response from the completion_queue.
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

//    GetSubmissionQueueSize call. Return the size of the submission_queue.
int LocalControllerSession::getSubmissionQueueSize ()
{
    return submission_queue_.size ();
}

//    SubmitRule call. Submit rules to the LocalControllerSession.
PStatus LocalControllerSession::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

//    GetResult call. Get results from the LocalControllerSession.
std::unique_ptr<StageResponse> LocalControllerSession::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

long LocalControllerSession::SessionIdentifier () const
{
    return session_id_;
}

} // namespace shepherd
