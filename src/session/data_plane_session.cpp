/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/session/data_plane_session.hpp>

namespace shepherd {

// DataPlaneSession default constructor.
DataPlaneSession::DataPlaneSession () :
    session_id_ { 0 },
    interface_ {},
    submission_queue_ {},
    completion_queue_ {}
{ }

// DataPlaneSession parameterized constructor.
DataPlaneSession::DataPlaneSession (long id) :
    session_id_ { id },
    interface_ {},
    submission_queue_ {},
    completion_queue_ {}
{ }

// DataPlaneSession default destructor.
DataPlaneSession::~DataPlaneSession () = default;

// StartSession() call. Start session between the controller and the data plane
// stage.
void DataPlaneSession::StartSession (int socket)
{
    Logging::log_debug ("DataPlaneStage::StartSession");

    Logging::log_debug ("DataPlaneSession :: " + std::to_string (getSubmissionQueueSize ()));

    int i = 1;

    // after knowing the Stage identifier,
    while (i) {
        PStatus status;
        ControlOperation operation {};
        std::string rule {};

        status = DequeueRuleFromSubmissionQueue (rule);

        status = SendRule (socket, rule, &operation);
    }
}

// SendRule call. Submit rule to the data plane stage.
PStatus DataPlaneSession::SendRule (int socket, const std::string& rule, ControlOperation* operation)
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
            status = interface_.mark_stage_ready (socket, operation, stage_ready, ack);
            // enqueue response of data plane stage from mar_stage_ready request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (STAGE_READY, ack.m_message));
            break;
        }

        case CREATE_HSK_RULE: {
            std::cout << "CREATE_HSK_RULE ... \n";
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateHousekeepingRule
            status = interface_.create_housekeeping_rule (socket, operation, rule, ack);
            // enqueue response of data plane stage from CreateHousekeepingRule request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (CREATE_HSK_RULE, ack.m_message));
            break;
        }

        case EXEC_HSK_RULES: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's ExecuteHousekeepingRule
            status = interface_.ExecuteHousekeepingRules (socket, operation, rule, ack);
            // enqueue response of data plane stage from ExecuteHousekeepingRule
            // request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (EXEC_HSK_RULES, ack.m_message));
            break;
        }

        case CREATE_ENF_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's CreateEnforcementRule
            status = interface_.create_enforcement_rule (socket, operation, rule, ack);

            // enqueue response of data plane stage from CreateEnforcementRule
            // request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (CREATE_ENF_RULE, ack.m_message));

            break;
        }

        case REMOVE_RULE: {
            // create temporary ACK structure
            ACK ack {};
            // invoke SouthboundInterface's RemoveRule
            status = interface_.RemoveRule (socket, operation, operation->m_operation_id, ack);
            // enqueue response of data plane stage from RemoveRule request
            EnqueueResponseInCompletionQueue (std::make_unique<StageResponseACK> (REMOVE_RULE, ack.m_message));
            break;
        }

        case COLLECT_STATS:
            status = interface_.collect_statistics (socket, operation);
            break;

        case COLLECT_GLOBAL_STATS: {
            // create temporary StatsGlobalRaw structure
            StatsGlobalRaw stats_global {};
            // invoke SouthboundInterface's CollectStatisticsKVS
            status = interface_.collect_global_statistics(socket, operation, stats_global);

            if (status.isOk ()) {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStatsGlobal> (COLLECT_GLOBAL_STATS,
                        stats_global.m_data_stats.m_read_rate,
                        stats_global.m_data_stats.m_write_rate,
                        stats_global.m_metadata_stats.m_open_rate,
                        stats_global.m_metadata_stats.m_close_rate,
                        stats_global.m_metadata_stats.m_getattr_rate,
                        stats_global.m_metadata_stats.m_metadata_total_rate));
            } else {
                // enqueue response of data plane stage from collect_tensorflow_statistics request
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStatsGlobal> (COLLECT_GLOBAL_STATS, -1, -1, -1, -1, -1, -1));
            }
            break;
        }

        case COLLECT_ENTITY_STATS: {
            // create temporary StatsEntityRaw structure
            StatsEntityRaw stats_entity {};

            // invoke SouthboundInterface's CollectStatisticsKVS
            status = interface_.collect_entity_statistics(socket, operation, stats_entity);

            std::unique_ptr<std::unordered_map<std::string, double>> stats_entities_object =
                std::make_unique<std::unordered_map<std::string, double>>();

            if (status.isOk ()) {
                int index = 1;
                for (auto stats_metadata : stats_entity.stats) {
                    stats_entities_object->emplace("mds" + std::to_string(index), stats_metadata);
                    index ++;
                }

                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStatsEntity> (COLLECT_ENTITY_STATS, stats_entities_object)
                );
            } else {
                EnqueueResponseInCompletionQueue (
                    std::make_unique<StageResponseStatsEntity> (COLLECT_ENTITY_STATS, stats_entities_object));
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
void DataPlaneSession::EnqueueRuleInSubmissionQueue (const std::string& rule)
{
    std::unique_lock<std::mutex> lock_t { submission_queue_lock_ };
    submission_queue_.emplace (rule);
    submission_queue_condition_.notify_one ();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the
//  execution ... DequeueRuleFromSubmissionQueue call. Dequeue rule from the submission_queue.
PStatus DataPlaneSession::DequeueRuleFromSubmissionQueue (std::string& rule)
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

//    SubmitRule call. Submit rules to the DataPlaneSession.
PStatus DataPlaneSession::SubmitRule (const std::string& submission_rule)
{
    PStatus status_t = PStatus::Error ();

    EnqueueRuleInSubmissionQueue (submission_rule);
    status_t = PStatus::OK ();

    return status_t;
}

//    GetResult call. Get results from the DataPlaneSession.
std::unique_ptr<StageResponse> DataPlaneSession::GetResult ()
{
    return DequeueResponseFromCompletionQueue ();
}

long DataPlaneSession::SessionIdentifier () const
{
    return session_id_;
}

} // namespace shepherd
