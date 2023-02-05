/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_CORE_CONTROL_APPLICATION_HPP
#define CHEFERD_CORE_CONTROL_APPLICATION_HPP

#include "cheferd/session/local_controller_session.hpp"
#include "control_application.hpp"

#include <regex>

namespace cheferd {

// Threshold to decide if a new rule should be enforced.
#define IOPS_THRESHOLD 10
// Number of rounds for aggregated statistics.
#define COLLECT_ROUNDS 5

/**
 * CoreControlApplication class.
 * The CoreControlApplication represents the core controller that coordinates the entire system.
 * Currently, the CoreControlApplication class contains the following variables:
 * - change_in_system: atomic value that marks if there is a change in the system (e.g., add/remove
 * local controller/data plane stage; new rule).
 * - pending_rules_queue_: queue that holds new rules submitted by the system administrator.
 * - pending_rules_queue_lock_: mutex for concurrency control over pending_rules_queue_.
 * - local_queue: queue that holds pending local controller trying to connect.
 * - pending_register_session_lock_: mutex for concurrency control over local_queue.
 * - local_to_data_queue_: queue that holds pending data plane sessions trying to connect.
 * - pending_register_stage_lock_: mutex for concurrency control over local_to_data_queue_.
 * - m_control_type: type of control that control application imposes (e.g., STATIC,
 * DYNAMIC_VANILLA).
 * - local_sessions_: container used for mapping a local controller identifier to
 * its LocalControllerSession.
 * - local_to_stages: container used for mapping a local controller identifier to
 * its current stages overviewed by it.
 * - stage_info_detailed: container used for mapping a stage to a Stageinfo struct that holds
 * its detailed information.
 * - job_location_tracker: container used for mapping a job to its local controllers.
 * - job_demands: container used for mapping a job's current demand.
 * - job_rates:  container used for mapping a job's current imposed rate.
 * - job_previous_rates: container used for mapping a job's previous imposed rate.
 * - maximum_limit: defines the maximum limit of the system (e.g., IOPS, bandwidth).
 * - active_ops: current operations supported by the controller.
 * - m_active_local_controller_sessions: atomic value that marks the number of active local
 * controller sessions.
 * - m_pending_local_controller_sessions: atomic value that marks the number of pending local
 * controller sessions.
 * - m_active_data_plane_sessions: atomic value that marks the number of active data
 * plane sessions.
 * - m_pending_data_plane_sessions: atomic value that marks the number of pending data
 * plane sessions.
 */
class CoreControlApplication : public ControlApplication {

private:
    std::atomic<bool> change_in_system;
    std::queue<std::string> pending_rules_queue_;
    std::mutex pending_rules_queue_lock_;
    std::queue<std::string> local_queue;
    std::mutex pending_register_session_lock_;
    std::queue<std::unique_ptr<StageInfo>> local_to_data_queue_;
    std::mutex pending_register_stage_lock_;
    ControlType m_control_type { ControlType::NOOP };
    std::unordered_map<std::string, std::unique_ptr<LocalControllerSession>> local_sessions_;
    std::unordered_map<std::string, std::vector<std::string>> local_to_stages;
    std::unordered_map<std::string, std::unique_ptr<StageInfo>> stage_info_detailed;
    std::map<std::string, std::unordered_map<std::string, std::vector<int>>> job_location_tracker;
    std::unordered_map<std::string, long> job_demands;
    std::unordered_map<std::string, long> job_rates;
    std::unordered_map<std::string, long> job_previous_rates;
    long maximum_limit;
    std::unordered_set<std::string> active_ops;
    std::atomic<int> m_active_local_controller_sessions;
    std::atomic<int> m_pending_local_controller_sessions;
    std::atomic<int> m_active_data_plane_sessions;
    std::atomic<int> m_pending_data_plane_sessions;

    /**
     * initialize: Fills housekeeping rules and operations supported by the control application.
     */
    void initialize ();

    /**
     * execute_feedback_loop: Executes feedback loop.
     * This method is responsible to verify if there is any pending local controller or
     * data plane trying to connect. And then performs a feedback loop that collects metrics from
     * the system, computes and enforces the appropriate limits, a sleeps until is time to start
     * the next loop.
     */
    void execute_feedback_loop ();

    /**
     * handle_local_controller_sessions. Processes pending local controller session.
     */
    void handle_local_controller_sessions ();

    /**
     * handle_data_plane_sessions. Processes pending data plane sessions.
     */
    void handle_data_plane_sessions ();

    /**
     * local_handshake: Performs a handshake with the local controller.
     * When a local controller connects to the core controller, it is informed of the housekeeping
     * rules that should be imposed at the data plane stages.
     * @param local_controller_address Local controller identifier.
     * @return Returns PStatus::OK if the handshake was successful, PStatus::Error otherwise.
     */
    PStatus local_handshake (const std::string& local_controller_address);

    /**
     * call_local_handshake: Submits LOCAL_HANDSHAKE rule to a local controller.
     * @param local_controller_address  Local controller identifier.
     * @return  Returns PStatus::OK if the handshake was successful, PStatus::Error otherwise.
     */
    PStatus call_local_handshake (const std::string& local_controller_address);

    /**
     * mark_data_plane_stage_ready: Submits STAGE_READY rule to a local controller to inform
     * that the data plane stage identified by stage_name and stage_env has been register
     * by the core controller.
     * @param local_controller_address Local controller identifier.
     * @param stage_name Stage's job name.
     * @param stage_env  Stage's env.
     * @return Returns PStatus::OK if successful, PStatus::Error otherwise.
     */
    PStatus mark_data_plane_stage_ready (const std::string& local_controller_address,
        const std::string& stage_name,
        const std::string& stage_env);

    /**
     * collect_statistics_global_send: Sends to local controllers the request to collect statistics.
     * @return Returns a list with the local controllers that the request was sent.
     */
    std::list<std::string> collect_statistics_global_send ();

    /**
     * collect_statistics_global_collect: Collects statistics from data plane stages.
     * @param sessions_sent List with the local controllers that the request was sent.
     * @return Returns the statistics collected.
     */
    std::unordered_map<std::string, std::unique_ptr<StageResponse>>
    collect_statistics_global_collect (std::list<std::string>& sessions_sent);

    /**
     * collect_statistics_global: Sends the request and collects statistics from data plane stages.
     * @return Returns the statistics collected.
     */
    std::unordered_map<std::string, std::unique_ptr<StageResponse>> collect_statistics_global ();

    /**
     * compute_and_enforce_static_rules: Computes and enforces STATIC policies.
     * @param d_stats Statistics collected from data plane stages.
     */
    void compute_and_enforce_static_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats);

    /**
     * compute_and_enforce_dynamic_vanilla_rules: Computes and enforces DYNAMIC_VANILLA policies.
     * @param d_stats Statistics collected from data plane stages.
     */
    void compute_and_enforce_dynamic_vanilla_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats);

    /**
     * compute_and_enforce_dynamic_leftover_rules: Computes and enforces DYNAMIC_LEFTOVER policies.
     * @param d_stats Statistics collected from data plane stages.
     */
    void compute_and_enforce_dynamic_leftover_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats,
        std::list<std::string>& sessions_sent);

    /**
     * sleep: Used to make control application main thread wait for the next loop.
     */
    void sleep () override;

    /**
     * dequeue_rule_from_queue: Dequeues a new rule submitted by the system administrator.
     * @return New rule.
     */
    std::string dequeue_rule_from_queue ();

    /**
     * update_job_demands. Processes new rules and updates job's demands.
     * @return
     */
    std::string update_job_demands ();

    /**
     * send_enforcement_rule. Sends new enforcement rules to local controllers.
     * @param app_name Job's name.
     * @param local_to_envs Container that stores job's location.
     * @param operation Operation that enforcement rule refers to.
     * @param job_address_updated Container that stores local controllers where
     * the new enforcement rules were submitted.
     */
    void send_enforcement_rule (std::string app_name,
        std::unordered_map<std::string, std::vector<int>> local_to_envs,
        std::string operation,
        std::unordered_map<std::string, bool>& job_address_updated);

    /**
     * collect_enforcement_rule_results. Collects enforcement rules results.
     * @param job_address_updated Container that stores local controllers where
     * the new enforcement rules were submitted.
     */
    void collect_enforcement_rule_results (
        std::unordered_map<std::string, bool>& job_address_updated);

    /**
     * collect_statistics_result. Collects a local controller statistics.
     * @param session LocalControllerSession to where collect results.
     * @param local_address  Local controller identifier.
     * @param sessions_to_delete Data plane sessions that are no longer operational
     * and should be deleted.
     * @param collected_stats Collected statistics.
     */
    void collect_statistics_result (const std::unique_ptr<LocalControllerSession>& session,
        std::string local_address,
        std::list<std::string>& sessions_to_delete,
        std::unordered_map<std::string, std::unique_ptr<StageResponse>>& collected_stats);

    /**
     * stage_name_env: Removes a data plane stage that is no longer operational.
     * @param stage_name_env Data plane stage identifier.
     */
    void remove_stage (const std::string& stage_name_env);

    /**
     * parse_rule_with_break: parses a rule into tokens using '|' as delimiter.
     * @param rule Rule to be parsed.
     * @param tokens Container to store parsed tokens.
     */
    void parse_rule_with_break (const std::string& rule, std::vector<std::string>* tokens);

public:
    /**
     * CoreControlApplication parameterized constructor.
     * @param control_type Type of control (STATIC, DYNAMIC_VANILLA, DYNAMIC_LEFTOVER).
     * @param rules_ptr Container that holds the housekeeping rules.
     * @param cycle_sleep_time Amount of time that a feedback-loop cycle should take.
     * @param maximum_limit Maximum allowed operations in the system (e.g., IOPS or bandwidth)
     */
    CoreControlApplication (ControlType control_type,
        std::vector<std::string>* rules_ptr,
        const uint64_t& cycle_sleep_time,
        long maximum_limit);

    /**
     * CoreControlApplication default destructor.
     */
    ~CoreControlApplication () override;

    /**
     * operator: Used to initiate the control application feedback loop execution.
     */
    void operator() () override;

    /**
     * register_local_controller_session: Register a new local controller.
     * @param local_controller_address Local controller identifier.
     */
    void register_local_controller_session (const std::string& local_controller_address);

    /**
     * register_stage_session: Register a new data plane.
     * @param local_controller_address Local controller identifier.
     * @param stage_name Data plane stage job's name.
     * @param stage_env Data plane stage job's env.
     * @param stage_user Data plane stage job's user.
     */
    void register_stage_session (const std::string& local_controller_address,
        const std::string& stage_name,
        const std::string& stage_env,
        const std::string& stage_user);

    /**
     * enqueue_rule_in_queue: Submit new rule to be imposed (used by the system administrator).
     * @param rule New rule.
     */
    void enqueue_rule_in_queue (const std::string& rule);

    /**
     * stop_feedback_loop: Stops the feedback loop from executing.
     */
    void stop_feedback_loop () override;
};
} // namespace cheferd

#endif // CHEFERD_CORE_CONTROL_APPLICATION_HPP
