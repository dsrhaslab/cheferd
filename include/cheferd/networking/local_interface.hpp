/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_LOCAL_INTERFACE_HPP
#define CHEFERD_LOCAL_INTERFACE_HPP

#include "cheferd/networking/stage_response/stage_response_stats.hpp"
#include "cheferd/networking/stage_response/stage_response_stat.hpp"

#include <cheferd/networking/southbound_interface.hpp>
#include <cheferd/utils/logging.hpp>
#include <cstdio>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <netinet/in.h>
#include <random>
#include <sstream>
#include <sys/un.h>
#include <unistd.h>

#ifdef BAZEL_BUILD
#include "examples/protos/controllers_grpc_interface.grpc.pb.h"
#else
#include "controllers_grpc_interface.grpc.pb.h"
#endif

using controllers_grpc_interface::ACK;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::ConnectRequest;
using controllers_grpc_interface::EnforcementOpRules;
using controllers_grpc_interface::EnforcementRules;
using controllers_grpc_interface::StageInfo;
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

/**
 * LocalInterface class.
 * ...
 */

class LocalInterface {

private:
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens, char c);

    void fill_housekeeping_rules_grpc (
        controllers_grpc_interface::LocalSimplifiedHandshakeRaw* housekeeping_rules,
        const std::string& rule);

    std::unique_ptr<GlobalToLocal::Stub> stub_;

public:
    /**
     * LocalInterface parameterized constructor.
     */
    explicit LocalInterface (const std::string& user_address);

    /**
     * LocalInterface default destructor.
     */
    ~LocalInterface ();

    /**
     * LocalHandshake: ...
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param response
     * @return
     */
    PStatus local_handshake (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * StageHandshake: ...
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param stage_handshake_object
     * @return
     */
    PStatus stage_handshake (const std::string& user_address,
        ControlOperation* operation,
        StageSimplifiedHandshakeRaw& stage_handshake_obj);

    /**
     * mark_stage_ready
     * @param user_address Corresponds to the local controller address.
     * @param operation
     * @param stage_ready_obj
     * @param response
     * @return
     */
    PStatus mark_stage_ready (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * CreateEnforcementRule: ...
     * @param user_address Corresponds to the local controller address.
     * @param send ...
     * @param rule ...
     * @param response ...
     * @return ...
     */
    PStatus create_enforcement_rule (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * CollectStatisticsTF: get the statistics of a PosixKVSInstance-based data
     * plane stage (highly-oriented to use case 2).
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_global_statistics (const std::string& user_address,
        ControlOperation* operation,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            stats_tf_objects);

    /**
     * CollectStatisticsTF: get the statistics of a PosixKVSInstance-based data
     * plane stage (highly-oriented to use case 2).
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_global_statistics_aggregated (const std::string& user_address,
        ControlOperation* operation,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            stats_tf_objects);
};
} // namespace cheferd

#endif // CHEFERD_LOCAL_INTERFACE_HPP
