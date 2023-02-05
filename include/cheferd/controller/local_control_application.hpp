/**
 *   Copyright (c) 2022 INESC TEC.
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

// Number of rounds for aggregated statistics.
#define COLLECT_ROUNDS 5

/**
 * LocalControlApplication class.
 * The LocalControlApplication represents the local controller that coordinates data plane stages.
 * Currently, the LocalControlApplication class contains the following variables:
 * - local_address: local controller address.
 * - data_sessions_: container used for mapping active data plane stages to its DataPlaneSession.
 * - preparing_data_sessions_: container used for mapping preparing data plane stages to its
 * DataPlaneSession.
 * - pending_data_sessions_: queue that holds pending data plane sessions.
 * - pending_data_plane_sessions_lock_: mutex for concurrency control over pending_data_sessions_.
 * - operation_to_channel_object: container used for mapping an operation to its respective channel
 * in the data plane stage context.
 * - core_stub_: unique_ptr of stub used to communicate with the core controller.
 * - server: unique_ptr of Server  used to communicate by the core controller.
 * - m_active_data_plane_sessions: atomic value that marks the number of active data
 * plane sessions.
 * - m_pending_data_plane_sessions: atomic value that marks the number of pending data
 * plane sessions.
 */
class LocalControlApplication : public GlobalToLocal::Service, public ControlApplication {

private:
    std::string local_address;
    std::unordered_map<std::string, std::unique_ptr<DataPlaneSession>> data_sessions_;
    std::unordered_map<std::string, std::unique_ptr<DataPlaneSession>> preparing_data_sessions_;
    std::queue<std::unique_ptr<HandshakeSession>> pending_data_sessions_;
    std::mutex pending_data_plane_sessions_lock_;
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> operation_to_channel_object;
    std::unique_ptr<LocalToGlobal::Stub> core_stub_;
    std::unique_ptr<Server> server;
    std::atomic<int> m_active_data_plane_sessions;
    std::atomic<int> m_pending_data_plane_sessions;

    /**
     * initialize: Initialize control application.
     */
    void initialize ();

    /**
     * execute_feedback_loop: Executes feedback loop.
     * This method is responsible to verify if there is any pending data plane trying to connect.
     */
    void execute_feedback_loop ();

    /**
     * handle_data_plane_sessions. Processes pending data plane sessions.
     */
    void handle_data_plane_sessions ();

    /**
     * sleep: Used to make control application main thread wait for the next loop.
     */
    void sleep () override;

    /**
     * call_stage_handshake: Submits STAGE_HANDSHAKE rule and housekeeping rules to data plane
     * stage.
     * @param handshake_session  Session to submit handshake rule to.
     * @return  Returns unique_ptr holding data plane stage detailed information.
     */
    std::unique_ptr<StageInfo> call_stage_handshake (HandshakeSession* handshake_session);

    /**
     * mark_stage_ready: Submits STAGE_READY to data plane stage.
     * @param stage_name_env Data plane stage identifier.
     * @return Returns PStatus::OK if successful, PStatus::Error otherwise.
     */
    PStatus mark_stage_ready (const std::string& stage_name_env) const;

    /**
     * submit_housekeeping_rules: Submits housekeeping rules to data plane stage.
     * @param stage_name_env Data plane stage identifier.
     * @return Number of housekeeping rules successfully submitted.
     */
    int submit_housekeeping_rules (const std::string& stage_name_env) const;

    /**
     * fill_socket_info: Defines a new individual socket for data plane stage.
     * @param handshake_ptr Stores data plane stage information.
     * @param socket_info New socket for data plane stage.
     * @return Returns PStatus::OK if successful, PStatus::Error otherwise.
     */
    static PStatus fill_socket_info (StageResponseHandshake* handshake_ptr,
        std::string& socket_info);

    /**
     * RunGlobalToLocalServer: Execute server to core controller communicate to local controller.
     */
    void RunGlobalToLocalServer ();

    /**
     * ConnectLocalToGlobal: Connect local controller to core controller.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status ConnectLocalToGlobal ();

    /**
     * ConnectStageToGlobal: Connect data plane stage  to core controller.
     * @param stage_name Data plane stage job's name.
     * @param stage_env Data plane stage job's env.
     * @param stage_user Data plane stage job's user.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status ConnectStageToGlobal (const std::string& stage_name,
        const std::string& stage_env,
        const std::string& stage_user);

    /**
     * LocalHandshake: Local controller handshake from core controller.
     * @param context Server context.
     * @param request Container that stores housekeeping rules.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status LocalHandshake (ServerContext* context,
        const controllers_grpc_interface::LocalSimplifiedHandshakeRaw* request,
        controllers_grpc_interface::ACK* reply) override;

    /**
     * StageHandshake: Stage handshake from core controller.
     * @param context Server context.
     * @param request Defines control operation.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status StageHandshake (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StageSimplifiedHandshakeRaw* reply) override;

    /**
     * MarkStageReady. Mark stage ready from core controller.
     * @param context Server context.
     * @param request Defines if stage is ready.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status MarkStageReady (ServerContext* context,
        const controllers_grpc_interface::StageReadyRaw* request,
        controllers_grpc_interface::ACK* reply) override;

    /**
     * CreateEnforcementRule: Create enforcement from core controller.
     * @param context Server context.
     * @param request Rules to be enforced.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status CreateEnforcementRule (ServerContext* context,
        const controllers_grpc_interface::EnforcementRules* request,
        controllers_grpc_interface::ACK* reply) override;

    /**
     * CollectGlobalStatistics: Collect Statistics request from core controller.
     * @param context Server context.
     * @param request Defines control operation.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status CollectGlobalStatistics (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        controllers_grpc_interface::StatsGlobalMap* reply) override;

    /**
     * CollectGlobalStatisticsAggregated: Collect Statistics request from core controller.
     * It aggregates the statistics from several rounds.
     * @param context Server context.
     * @param request Defines control operation.
     * @param reply Response.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status CollectGlobalStatisticsAggregated (ServerContext* context,
        const controllers_grpc_interface::ControlOperation* request,
        StatsGlobalMap* reply) override;

    /**
     * LocalPassthru: General function to submit rules to data plane stages.
     * @param stage_name_env Data plane stage identifier.
     * @param rule Rule to be submitted.
     * @return Returns Status::OK if successful, Status::Error otherwise.
     */
    Status LocalPassthru (std::string stage_name_env, std::string rule);

public:
    /**
     * LocalControlApplication parameterized constructor.
     * @param core_address Core controller address.
     * @param local_address Local controller address.
     */
    LocalControlApplication (const std::string& core_address, const std::string& local_address);

    /**
     * LocalControlApplication parameterized constructor.
     * @param rules_ptr Container that holds the housekeeping rules.
     * @param core_address Core controller address.
     * @param local_address Local controller address.
     * @param cycle_sleep_time Amount of time that a feedback-loop cycle should take.
     */
    LocalControlApplication (std::vector<std::string>* rules_ptr,
        const std::string& core_address,
        const std::string& local_address,
        const uint64_t& cycle_sleep_time);

    /**
     * LocalControlApplication default destructor.
     */
    ~LocalControlApplication () override;

    /**
     * operator: Used to initiate the control application feedback loop execution.
     */
    void operator() () override;

    /**
     * register_stage_session: Register a new data plane.
     * @param socket_t Socket id.
     */
    void register_stage_session (int socket_t);

    /**
     * stop_feedback_loop: Stops the feedback loop from executing.
     */
    void stop_feedback_loop () override;

    /**
     * parse_rule: parses a rule into tokens using char c as delimiter.
     * @param rule Rule to be parsed.
     * @param tokens Container to store parsed tokens.
     * @param c Delimiter.
     */
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens, char c);
};

} // namespace cheferd

#endif // CHEFERD_LOCAL_CONTROL_APPLICATION_HPP
