/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_CORE_CONTROL_APPLICATION_HPP
#define SHEPHERD_CORE_CONTROL_APPLICATION_HPP

#include <shepherd/controller/control_application.hpp>
#include <regex>

namespace shepherd {

#define MAX_TENSORFLOW_INSTANCES 4
#define TENSORFLOW_STATISTICS    (MAX_TENSORFLOW_INSTANCES + 1)
//#define TOKEN_BUCKET_THRESHOLD   10485760 // 10 MB/S

#define IOPS_THRESHOLD           10
#define TOKEN_BUCKET_THRESHOLD   5

/**
 * PidIOStats: Structure to hold the I/O statistics (read and write throughput)
 * of a given process.
 */
struct PidIOStats {
    int m_pid;
    double m_read_thr;
    double m_write_thr;
};

/**
 * StatsTFControlApplication: structure that contains the I/O statistics of a
 * TensorFlow data plane stage and the respective PID stat statistics (read and
 * write bandwidth on disk).
 */
struct StatsTFControlApplication {
    double m_instance_read_bandwidth = 0;
    double m_instance_write_bandwidth = 0;
    double m_pid_read_bandwidth = 0;
    double m_pid_write_bandwidth = 0;
};


/**
 * StatsTFControlApplication: structure that contains the I/O statistics of a
 * TensorFlow data plane stage and the respective PID stat statistics (read and
 * write bandwidth on disk).
 */
struct StatsControlApplicationGlobal {
    double m_instance_read_rate = 0;
    double m_instance_write_rate = 0;
    double m_instance_open_rate = 0;
    double m_instance_close_rate = 0;
    double m_instance_getattr_rate = 0;
    double m_instance_metadata_total_rate = 0;
};

struct StatsControlApplicationEntity {
    std::unordered_map<std::string, double> m_total;
};


/**
 * CoreControlApplication class.
 * Complete me ...
 */
class CoreControlApplication : public ControlApplication {

private:

    int m_token_bucket_calibration[MAX_TENSORFLOW_INSTANCES] { };

    long MAX_BANDWIDTH_BPS { option_max_bandwidth_bps };
    std::queue<std::string>pending_rules_queue_;
    std::mutex pending_rules_queue_lock_;

    ControlType m_control_type { ControlType::NOOP };
    std::vector<std::unique_ptr<LocalControllerSession>> local_sessions_;
    std::unordered_map<std::string, std::unordered_map<int, std::vector<int>>> job_location_tracker;
    std::unordered_map<std::string, int> job_rates;
    std::unordered_map<std::string, int> job_previous_rates;
    std::unordered_map<std::string, int> job_demands;

    std::queue<std::pair<int,std::string>>local_to_data_queue_;
    long maximum_iops;



    /**
     * initialize:
     *
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
    PStatus local_handshake (int local_index);

    // FIXME: not using this method ...
    /**
     * CollectStatistics:
     * @return
     */
    std::unique_ptr<StageResponse> collect_statistics () override;

    /**
     * CollectStatistics:
     * @param active_sessions
     * @param start_index
     * @return
     */
    std::unordered_map<int, std::unique_ptr<StageResponse>>
    collect_statistics_global (const int& active_sessions, const int& start_index);

    std::unordered_map<int, std::unique_ptr<StageResponse>>
    collect_statistics_entity (const int& active_sessions, const int& start_index);

    /**
     * Compute:
     * @param statistics_ptr
     */
    void compute_and_enforce_static_rules (const std::unordered_map<int, std::unique_ptr<StageResponse>>& s_stats,
        const int& active_sessions,
        const int& start_index);

    void compute_and_enforce_dynamic_rules (const std::unordered_map<int, std::unique_ptr<StageResponse>>& d_stats,
        const int& active_sessions,
        const int& start_index);

    void compute_and_enforce_mds_rules (const std::unordered_map<int, std::unique_ptr<StageResponse>>& entity_stats,
        const int& active_sessions, const int& start_index);

    std::string DequeueRuleFromQueue ();

    // FIXME: not using this method ...
    /**
     * Compute:
     * @param statistics_ptr
     */
    void compute (const std::unique_ptr<StageResponse>& statistics_ptr) override;

    /**
     * Enforce:
     * @param active_sessions
     * @param start_index
     */
    void enforce (int active_sessions, int start_index);

    /**
     * Sleep:
     */
    void sleep () override;

    /**
     * CallLocalHandshake:
     * @return
     */
    PStatus call_local_handshake (const int& local_index);


    /**
     * CalibrateStageRate: ...
     * @param stage_statistic
     * @param pid_statistic
     * @return
     */
    double calibrate_stage_rate_factor (const int& index,
        const double& stage_statistic,
        const double& pid_statistic);

public:
    CoreControlApplication (ControlType control_type);

    CoreControlApplication (ControlType control_type,
        std::vector<std::string>* rules_ptr,
        const uint64_t& cycle_sleep_time);

    ~CoreControlApplication () override;

    void operator() () override;

    LocalControllerSession* register_local_controller_session (const std::string& local_controller_address,
        int local_index);

    void register_stage_session (const std::string& local_controller_address,
        const std::string& stage_name,
        const std::string& stage_env,
        const std::string& stage_user);

    void EnqueueRuleInQueue (std::string rule);
};
} // namespace shepherd

#endif // SHEPHERD_CORE_CONTROL_APPLICATION_HPP
