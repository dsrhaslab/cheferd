/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_CORE_CONTROL_APPLICATION_HPP
#define CHEFERD_CORE_CONTROL_APPLICATION_HPP

#include "cheferd/session/local_controller_session.hpp"
#include "control_application.hpp"

#include <regex>

namespace cheferd {

// Threshold to decide if a new should be enforced.
#define IOPS_THRESHOLD 10


/**
 * CoreControlApplication class.
 * The CoreControlApplication represents the global controller that coordinates the entire system.
 */
class CoreControlApplication : public ControlApplication {

private:
    // Change in the system (e.g., add/remove local controller/data plane stage; new rule).
    std::atomic<bool> change_in_system;

    // Related to the submission of new rules by the system administrator.
    std::queue<std::string> pending_rules_queue_;
    std::mutex pending_rules_queue_lock_;

    // Related to the registration of new local controller sessions.
    std::mutex pending_register_session_lock_;
    std::queue<std::string> local_queue;

    // Related to the registration of new data plane stages.
    std::mutex pending_register_stage_lock_;
    std::queue<std::unique_ptr<StageInfo>>
        local_to_data_queue_;

    // Type of control (e.g., Static, Dynamic, ...).
    ControlType m_control_type { ControlType::NOOP };

    // LocalControllerID -> LocalControlSession. Holds local controllers sessions.
    // (e.g., <"0.0.0.0:50052", LocalControlSession>)
    std::unordered_map<std::string, std::unique_ptr<LocalControllerSession>> local_sessions_;

    // LocalControllerID -> [StageID1, StageID2]. Holds info about each stage overviewed by a local
    // controller. (e.g., <"0.0.0.0:50052", ["job1+1", "job1+2"]>)
    std::unordered_map<std::string, std::vector<std::string>> local_to_stages;

    // StageID -> StageInfo. Details each stage information.
    // (e.g., <"job1+1", StageInfo>)
    std::unordered_map<std::string, std::unique_ptr<StageInfo>> stage_info_detailed;

    // AppID -> < LocalControllerID -> [Envs] >. Holds info about the location and  of each job
    // instance. (e.g., <"job1", <"0.0.0.0:50052", [1,2]>>
    std::map<std::string, std::unordered_map<std::string, std::vector<int>>> job_location_tracker;

    // AppID -> Value. Holds each job demand, rate, and previous rates.
    // (e.g., <"job1", 10000>
    std::unordered_map<std::string, long> job_demands;
    std::unordered_map<std::string, long> job_rates;
    std::unordered_map<std::string, long> job_previous_rates;

    // Maximum allowed limit in the system.
    long maximum_limit;

    // Operations supported by the controller.
    std::unordered_set<std::string> active_ops;

    std::atomic<int> m_active_local_controller_sessions;
    std::atomic<int> m_pending_local_controller_sessions;
    std::atomic<int> m_active_data_plane_sessions;
    std::atomic<int> m_pending_data_plane_sessions;


    /**
     * initialize: Fills housekeeping rules and operations supported by control application.
     *
     */
    void initialize ();

    /**
     * execute_feedback_loop:
     */
    void execute_feedback_loop ();

    /**
     * LocalControllerSessionHandshake:
     * @param index
     * @return
     */
    PStatus local_handshake (const std::string& local_controller_address);

    /**
     * CollectStatistics:
     * @param active_sessions
     * @param start_index
     * @return
     */

    std::list<std::string> collect_statistics_global_send ();

    std::unordered_map<std::string, std::unique_ptr<StageResponse>>
    collect_statistics_global_collect (std::list<std::string>& sessions_sent);

    std::unordered_map<std::string, std::unique_ptr<StageResponse>> collect_statistics_global ();

    /**
     * Compute:
     * @param statistics_ptr
     */
    void compute_and_enforce_static_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats);

    void compute_and_enforce_dynamic_vanilla_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats);

    void compute_and_enforce_dynamic_leftover_rules (
        const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats,
        std::list<std::string>& sessions_sent);

    std::string dequeue_rule_from_queue ();

    /**
     * Sleep:
     */
    void sleep () override;

    /**
     * CallLocalHandshake:
     * @return
     */
    PStatus call_local_handshake (const std::string& local_controller_address);

    PStatus mark_data_plane_stage_ready (const std::string& local_controller_address,
        const std::string& stage_name,
        const std::string& stage_env);

    void parse_rule_with_break (const std::string& rule, std::vector<std::string>* tokens);

public:
    CoreControlApplication (ControlType control_type,
        std::vector<std::string>* rules_ptr,
        const uint64_t& cycle_sleep_time,
        long maximum_limit);

    ~CoreControlApplication () override;

    void operator() () override;

    void register_local_controller_session (
        const std::string& local_controller_address);

    void register_stage_session (const std::string& local_controller_address,
        const std::string& stage_name,
        const std::string& stage_env,
        const std::string& stage_user);

    void enqueue_rule_in_queue (const std::string& rule);

    void stop_feedback_loop () override;

    std::string update_job_demands ();

    void send_enforcement_rule (std::string app_name,
        std::unordered_map<std::string, std::vector<int>> local_to_envs,
        std::string operation,
        std::unordered_map<std::string, bool>& job_address_updated);

    void collect_enforcement_rule_results (
        std::unordered_map<std::string, bool>& job_address_updated);

    void remove_stage (const std::string& stage_name_env);

    void collect_statistics_result (const std::unique_ptr<LocalControllerSession>& session,
        std::string local_address,
        std::list<std::string>& sessions_to_delete,
        std::unordered_map<std::string, std::unique_ptr<StageResponse>>& collected_stats);

    void handle_local_controller_sessions ();

    void handle_data_plane_sessions ();
};
} // namespace cheferd

#endif // CHEFERD_CORE_CONTROL_APPLICATION_HPP
