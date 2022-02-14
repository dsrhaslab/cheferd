/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_LOCAL_CONTROL_APPLICATION_HPP
#define SHEPHERD_LOCAL_CONTROL_APPLICATION_HPP

#include <regex>
#include <shepherd/controller/control_application.hpp>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/controllers_grpc_interface.grpc.pb.h"
#else
#include "controllers_grpc_interface.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using controllers_grpc_interface::ConnectRequest;
using controllers_grpc_interface::ConnectRequestStage;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::StageReadyRaw;
using controllers_grpc_interface::ACK;
using controllers_grpc_interface::HousekeepingCreateChannelString;
using controllers_grpc_interface::HousekeepingCreateObjectString;
using controllers_grpc_interface::Execute;
using controllers_grpc_interface::EnforcementRuleString;
using controllers_grpc_interface::StatsEntity;
using controllers_grpc_interface::StatsEntityMap;
using controllers_grpc_interface::StatsGlobalMap;


using controllers_grpc_interface::GlobalToLocal;
using controllers_grpc_interface::LocalToGlobal;
using controllers_grpc_interface::ControlOperation;
using controllers_grpc_interface::StageSimplifiedHandshakeRaw;
using controllers_grpc_interface::LocalSimplifiedHandshakeRaw;
using grpc::Channel;
using grpc::ClientContext;

namespace shepherd {

#define MAX_TENSORFLOW_INSTANCES 4
#define TENSORFLOW_STATISTICS    (MAX_TENSORFLOW_INSTANCES + 1)


/**
 * PidIOStats: Structure to hold the I/O statistics (read and write throughput)
 * of a given process.
 */
struct PidIOStats2 {
    int m_pid;
    double m_read_thr;
    double m_write_thr;
};


/**
 * LocalControlApplication class.
 * Complete me ...
 */
class LocalControlApplication : public GlobalToLocal::Service, public ControlApplication {

private:
    long m_previous_read_bytes_pid[TENSORFLOW_STATISTICS] { };
    long m_previous_write_bytes_pid[TENSORFLOW_STATISTICS] { };

    std::string core_address;
    std::string local_address;
    std::vector<std::unique_ptr<DataPlaneSession>> data_sessions_;
    std::unordered_map<std::string, int> stage_name_env_to_index_;
    std::unordered_map<int, int> index_to_pid_;
    std::unordered_map<int, std::pair<std::string, std::string>> index_to_stage_name_env_;
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> operation_to_channel_object;

    std::unique_ptr<LocalToGlobal::Stub> core_stub_;


    /**
     * initialize:
     */
    void initialize ();

    /**
     * execute_feedback_loop:
     */
    void execute_feedback_loop ();

    /**
     * DataPlaneSessionHandshake:
     * @param index
     * @return
     */
    PStatus stage_handshake (int index);

    /**
     * mark_data_plane_stage_ready
     * @param index
     * @return
     */
    PStatus mark_data_plane_stage_ready (const int& index);

    /**
     * CollectPidStats:
     * Use a static/fixed value for the time interval.
     * More information PID-based statistic collection in:
     * (1) https://github.com/sysstat/sysstat/blob/master/pidstat.c
     * (2)
     * https://www.mjmwired.net/kernel/Documentation/filesystems/proc.txt#1585
     * @param pid
     * @param previous_read_bytes
     * @param previous_write_bytes
     * @return
     */
    PidIOStats2 collect_pid_stats (const int& index);

    // FIXME: not using this method ...
    /**
     * CollectStatistics:
     * @return
     */
    std::unique_ptr<StageResponse> collect_statistics () override;

    // FIXME: not using this method ...
    /**
     * Compute:
     * @param statistics_ptr
     */
    void compute (const std::unique_ptr<StageResponse>& statistics_ptr) override;


    /**
     * Sleep:
     */
    void sleep () override;

    /**
     * CallStageHandshake:
     * @return
     */
    std::tuple<const std::string, const std::string, const std::string> call_stage_handshake (const int& index);

    /*
     * SubmitHousekeepingRules:
     * @return
     */
    int submit_housekeeping_rules (const int& index) const;


    void RunGlobalToLocalServer();
    Status ConnectLocalToGlobal();
    Status ConnectStageToGlobal(const std::string& stage_name, const std::string& stage_env, const std::string& stage_user);


    /*Requests from the CORE controller*/
    // Logic and data behind the server's behavior.
    Status LocalHandshake(ServerContext* context, const controllers_grpc_interface::LocalSimplifiedHandshakeRaw* request,
        controllers_grpc_interface::ACK* reply) override;
    Status StageHandshake(ServerContext* context, const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StageSimplifiedHandshakeRaw* reply) override;

    Status MarkStageReady(ServerContext* context, const controllers_grpc_interface::StageReadyRaw* request,
        controllers_grpc_interface::ACK* reply) override;

    Status CreateHouseKeepingRuleChannel(ServerContext* context, const controllers_grpc_interface::HousekeepingCreateChannelString* request,
        controllers_grpc_interface::ACK* reply) override;
    Status CreateHouseKeepingRuleObject(ServerContext* context, const controllers_grpc_interface::HousekeepingCreateObjectString* request,
        controllers_grpc_interface::ACK* reply) override;

    Status ExecuteHousekeepingRules(ServerContext* context, const controllers_grpc_interface::Execute* request,
        controllers_grpc_interface::ACK* reply) override;

    Status CreateEnforcementRule(ServerContext* context, const controllers_grpc_interface::EnforcementRuleString* request,
        controllers_grpc_interface::ACK* reply) override;

    Status CollectGlobalStatistics(ServerContext* context, const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StatsGlobalMap* reply) override;

    Status CollectEntityStatistics(ServerContext* context, const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StatsEntityMap* reply) override;

    Status LocalPassthru(const std::string stage_name, const std::string stage_env, const std::string rule);


public:
    LocalControlApplication (
        const std::string& core_address,
        const std::string& local_address
        );

    LocalControlApplication (
        std::vector<std::string>* rules_ptr,
        const std::string& core_address,
        const std::string& local_address,
        const uint64_t& cycle_sleep_time);

    ~LocalControlApplication () override;

    void operator() () override;

    //TODO: FIX this in control application parent class
    DataPlaneSession* register_stage_session (int index);

    void parse_rule (const std::string& rule, std::vector<std::string>* tokens, const char c);
};
} // namespace shepherd

#endif // SHEPHERD_LOCAL_CONTROL_APPLICATION_HPP
