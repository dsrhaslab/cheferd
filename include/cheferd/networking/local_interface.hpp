/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_LOCAL_INTERFACE_HPP
#define CHEFERD_LOCAL_INTERFACE_HPP

#include "cheferd/networking/stage_response/stage_response_stat.hpp"
#include "cheferd/networking/stage_response/stage_response_stats.hpp"

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

/**
 * PAIOInterface class.
 * Interface to communication with a local controller
 */
class LocalInterface {

private:
    /**
     * parse_rule: Parse a rule into tokens using char c as delimiter.
     * @param rule Rule to be parsed.
     * @param tokens Container to store parsed tokens.
     * @param c Delimiter.
     */
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens, char c);

    /**
     * fill_create_channel_rule: Fill LocalSimplifiedHandshakeRaw with rule data.
     * @param housekeeping_rules LocalSimplifiedHandshakeRaw object to be filled.
     * @param rule Data to fill object.
     */
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
     * local_handshake: Performs a handshake with the local controller. Informs local controller
     * of the housekeeping rules that should be imposed at the data plane stages.
     * @param user_address Corresponds to the local controller address.
     * @param operation ControlOperation.
     * @param rule Housekeeping rules.
     * @param response Response obtained.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus local_handshake (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * stage_handshake: Handshake a data plane stage.
     * Submit a handshake request to collect data about the data plane stage.
     * @param user_address Corresponds to the data plane address.
     * @param operation ControlOperation.
     * @param stage_handshake_obj StageSimplifiedHandshakeRaw object stores data plane stage
     * detailed information.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus stage_handshake (const std::string& user_address,
        ControlOperation* operation,
        StageSimplifiedHandshakeRaw& stage_handshake_obj);

    /**
     * mark_stage_ready: Mark data plane stage as ready.
     * @param user_address Corresponds to the local controller address.
     * @param operation ControlOperation.
     * @param rule Rule to mark stage as ready.
     * @param response Response obtained.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus mark_stage_ready (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * create_enforcement_rule: Submit enforcement rules to the local controller pass to its
     * data plane stages.
     * @param user_address  Corresponds to the local controller address.
     * @param operation  ControlOperation.
     * @param rule Enforcement rules.
     * @param response Response obtained.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus create_enforcement_rule (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * collect_global_statistics: Collect statistics from data plane stages.
     * @param user_address Corresponds to the local controller address.
     * @param operation ControlOperation.
     * @param stats_tf_objects Container to store responses.
     * @return  PStatus value that defines if the operation was successful.
     */
    PStatus collect_global_statistics (const std::string& user_address,
        ControlOperation* operation,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            stats_tf_objects);

    /**
     * collect_global_statistics_aggregated: Collect aggregated statistics from data plane stages.
     * @param user_address Corresponds to the local controller address.
     * @param operation ControlOperation.
     * @param stats_tf_objects Container to store responses.
     * @return  PStatus value that defines if the operation was successful.
     */
    PStatus collect_global_statistics_aggregated (const std::string& user_address,
        ControlOperation* operation,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            stats_tf_objects);
};
} // namespace cheferd

#endif // CHEFERD_LOCAL_INTERFACE_HPP
