/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_LOCAL_CONTROL_APPLICATION_HPP
#define CHEFERD_LOCAL_CONTROL_APPLICATION_HPP

#include <cheferd/controller/control_application.hpp>
#include <cheferd/session/data_plane_session.hpp>
#include <cheferd/session/handshake_session.hpp>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <regex>
#include <thread>

#ifdef BAZEL_BUILD
#include "examples/protos/controllers_grpc_interface.grpc.pb.h"
#else
#include "controllers_grpc_interface.grpc.pb.h"
#endif

using controllers_grpc_interface::ACK;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::ConnectRequest;
using controllers_grpc_interface::StageInfoConnect;
using controllers_grpc_interface::StageReadyRaw;
using controllers_grpc_interface::StatsGlobalMap;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using controllers_grpc_interface::ControlOperation;
using controllers_grpc_interface::GlobalToLocal;
using controllers_grpc_interface::LocalSimplifiedHandshakeRaw;
using controllers_grpc_interface::LocalToGlobal;
using controllers_grpc_interface::StageSimplifiedHandshakeRaw;
using grpc::Channel;
using grpc::ClientContext;

namespace cheferd {

#define COLLECT_ROUNDS 5

/**
 * LocalControlApplication class.
 * Complete me ...
 */
class LocalControlApplication : public GlobalToLocal::Service, public ControlApplication {

private:
    std::string local_address;

    std::unordered_map<std::string, std::unique_ptr<DataPlaneSession>> data_sessions_;
    std::unordered_map<std::string, std::unique_ptr<DataPlaneSession>> preparing_data_sessions_;

    // Related to the registration of new local controller sessions.
    std::mutex pending_data_plane_sessions_lock_;
    std::queue<std::unique_ptr<HandshakeSession>> pending_data_sessions_;


    std::unordered_map<std::string, std::vector<std::pair<int, int>>> operation_to_channel_object;

    std::unique_ptr<LocalToGlobal::Stub> core_stub_;

    std::unique_ptr<Server> server;

    std::atomic<int> m_active_data_plane_sessions;
    std::atomic<int> m_pending_data_plane_sessions;

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
    void handle_data_plane_sessions ();

    /**
     * Sleep:
     */
    void sleep () override;

    /**
     * CallStageHandshake:
     * @return
     */
    std::unique_ptr<StageInfo> call_stage_handshake (
        HandshakeSession* handshake_session);

    /*
     * SubmitHousekeepingRules:
     * @return
     */
    int submit_housekeeping_rules (const std::string& stage_name_env) const;

    static PStatus fill_socket_info (StageResponseHandshake* handshake_ptr,
        std::string& socket_info);

    void RunGlobalToLocalServer ();
    Status ConnectLocalToGlobal ();
    Status ConnectStageToGlobal (const std::string& stage_name,
        const std::string& stage_env,
        const std::string& stage_user);

    /*Requests from the CORE controller*/
    // Logic and data behind the server's behavior.
    Status LocalHandshake (ServerContext* context,
        const controllers_grpc_interface::LocalSimplifiedHandshakeRaw* request,
        controllers_grpc_interface::ACK* reply) override;
    Status StageHandshake (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StageSimplifiedHandshakeRaw* reply) override;

    Status MarkStageReady (ServerContext* context,
        const controllers_grpc_interface::StageReadyRaw* request,
        controllers_grpc_interface::ACK* reply) override;

    Status CreateEnforcementRule (ServerContext* context,
        const controllers_grpc_interface::EnforcementRules* request,
        controllers_grpc_interface::ACK* reply) override;

    Status CollectGlobalStatistics (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StatsGlobalMap* reply) override;

    Status CollectGlobalStatisticsAggregated (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        StatsGlobalMap* reply) override;

    Status LocalPassthru (std::string stage_name_env, std::string rule);

public:
    LocalControlApplication (const std::string& core_address, const std::string& local_address);

    LocalControlApplication (std::vector<std::string>* rules_ptr,
        const std::string& core_address,
        const std::string& local_address,
        const uint64_t& cycle_sleep_time);

    ~LocalControlApplication () override;

    void operator() () override;

    // TODO: FIX this in control application parent class
    void register_stage_session (int socket_t);

    void stop_feedback_loop () override;

    void parse_rule (const std::string& rule, std::vector<std::string>* tokens, char c);
    PStatus mark_stage_ready (const std::string& stage_name_env) const;
};

} // namespace cheferd

#endif // CHEFERD_LOCAL_CONTROL_APPLICATION_HPP
