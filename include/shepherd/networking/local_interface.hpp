/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_LOCAL_INTERFACE_HPP
#define SHEPHERD_LOCAL_INTERFACE_HPP

#include <cstdio>
#include <netinet/in.h>
#include <random>
#include <shepherd/networking/southbound_interface.hpp>
#include <shepherd/utils/logging.hpp>
#include <sys/un.h>
#include <unistd.h>
#include <sstream>
#include "shepherd/networking/stage_response/stage_response_stats.hpp"
#include "shepherd/networking/stage_response/stage_response_stats_global.hpp"
#include "shepherd/networking/stage_response/stage_response_stats_entity.hpp"

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
using controllers_grpc_interface::StageInfo;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::StageReadyRaw;
using controllers_grpc_interface::ACK;
using controllers_grpc_interface::HousekeepingCreateChannelString;
using controllers_grpc_interface::HousekeepingCreateObjectString;
using controllers_grpc_interface::HousekeepingCreateObjectRaw;
using controllers_grpc_interface::Execute;
using controllers_grpc_interface::EnforcementRuleString;
using controllers_grpc_interface::EnforcementRuleRaw;
using controllers_grpc_interface::StatsGlobalMap;
using controllers_grpc_interface::StatsEntityMap;


using controllers_grpc_interface::GlobalToLocal;
using controllers_grpc_interface::LocalToGlobal;
using controllers_grpc_interface::ControlOperation;
using controllers_grpc_interface::StageSimplifiedHandshakeRaw;
using controllers_grpc_interface::LocalSimplifiedHandshakeRaw;
using grpc::Channel;
using grpc::ClientContext;



namespace shepherd {

/**
 * LocalInterface class.
 * ...
 */


class LocalInterface {

private:
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens);

    void fill_housekeeping_rules_grpc(controllers_grpc_interface::LocalSimplifiedHandshakeRaw* housekeeping_rules,
                                      const std::string& rule);

    void fill_create_channel_rule (HousekeepingCreateChannelRaw* hsk_channel_obj,
        const std::vector<std::string>& tokens);

    void fill_create_channel_rule_grpc (controllers_grpc_interface::HousekeepingCreateChannelRaw* hsk_channel_obj,
                                   const std::vector<std::string>& tokens);

    void fill_create_object_rule (HousekeepingCreateObjectRaw* hsk_object_obj,
        const std::vector<std::string>& tokens);

    void fill_create_object_rule_grpc (controllers_grpc_interface::HousekeepingCreateObjectRaw* hsk_object_obj,
                                  const std::vector<std::string>& tokens);

    // void fillHousekeepingAssign (HousekeepingAssignRaw* hsk_object, const
    // std::vector<std::string>& tokens);

    void fill_enforcement_rule (EnforcementRuleRaw* enf_object,
        const std::vector<std::string>& tokens);

    void fill_enforcement_rule_grpc (controllers_grpc_interface::EnforcementRuleRaw* enf_object,
                                const std::vector<std::string>& tokens);

    std::unique_ptr<GlobalToLocal::Stub> stub_;

public:
    /**
     * LocalInterface parameterized constructor.
     */
    LocalInterface(const std::string& user_address);

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
     * CreateHousekeepingRule: Create a HousekeepingRule to be installed at the
     * data plane stage.
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param rule ...
     * @param response ...
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus create_housekeeping_rule (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * ExecuteHousekeepingRules: order to execute all pending HousekeepingRules.
     * @param user_address Corresponds to the local controller address.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param rule ...
     * @param response ...
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus ExecuteHousekeepingRules (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

    /**
     * CreateDifferentiationRule: ...
     * @param user_address Corresponds to the local controller address.
     * @param send  ...
     * @param rule  ...
     * @param response  ...
     * @return  ...
     */
    PStatus CreateDifferentiationRule (const std::string& user_address,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response);

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
     * RemoveRule: remove a HousekeepingRule from a specific data plane stage.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param user_address Corresponds to the local controller address.
     * @param rule_id HousekeepingRule identifier.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus
    RemoveRule (const std::string& user_address, ControlOperation* operation, int rule_id, ACK& response);

    /**
     * collect_statistics: get the statistics of a specific Enforcement Unit of
     * the current data plane stage.
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_statistics (const std::string& user_address, ControlOperation* operation);

    //TODO: Update the documentation
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
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& stats_tf_objects);

    /**
     * CollectStatisticsTF: get the statistics of a PosixKVSInstance-based data
     * plane stage (highly-oriented to use case 2).
     * @param user_address Corresponds to the local controller address.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_entity_statistics (const std::string& user_address,
        ControlOperation* operation,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& stats_tf_objects);
};
} // namespace shepherd

#endif // SHEPHERD_LOCAL_INTERFACE_HPP
