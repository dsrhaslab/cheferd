/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_INTERFACE_DEFINITIONS_HPP
#define CHEFERD_INTERFACE_DEFINITIONS_HPP

#include <cheferd/utils/context_propagation_definitions.hpp>
#include <climits>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace cheferd {

#define STAGE_HANDSHAKE        0
#define STAGE_READY            1
#define COLLECT_STATS          2
#define COLLECT_DETAILED_STATS 3
#define CREATE_HSK_RULE        4
#define CREATE_DIF_RULE        5
#define CREATE_ENF_RULE        6
#define EXEC_HSK_RULES         7
#define REMOVE_RULE            8

#define LOCAL_HANDSHAKE      11
#define STAGE_HANDSHAKE_INFO 12
#define COLLECT_ENTITY_STATS 14

#define HSK_CREATE_CHANNEL              1
#define HSK_CREATE_OBJECT               2
#define COLLECT_GLOBAL_STATS            5
#define COLLECT_GLOBAL_STATS_AGGREGATED 6

/**
 * ControlOperation structure.
 * Defines the metadata of the operation to be sent to the data plane stage.
 *  - m_operation_id: defines the control operation identifier;
 *  - m_operation_type: defines the type of operation to be received (housekeeping rule, enforcement
 *  rule, collect statistics, ...);
 *  - m_operation-subtype: defines the subtype of the operation to be received (create channel,
 *  create object, ...);
 *  - m_size: defines the size of the object to be received.
 */
struct ControlOperation {
    int m_operation_id { -1 };
    int m_operation_type { -1 };
    int m_operation_subtype { -1 };
    int m_size { -1 };
};

/**
 * ControlResponse structure.
 * Defines the metadata for submitting messages to the data plane stage.
 */
struct ControlResponse {
    int m_response;
};

/**
 * Acknowledgement codes.
 * Defines the codes for the acknowledgment messages for the communication between the control plane
 * and data plane stage.
 */
enum class AckCode { ok = 1, error = 0 };

/**
 * ACK structure. Defines if a command was executed or not, returning ACK_OK if
 * successfully executed, and ACK_ERROR otherwise.
 */
struct ACK {
    int m_message;
};

/**
 * Execute structure. Needs to be updated.
 */
struct Execute {
    bool execute_all;
};

/**
 * stage_name_max_size: defines the maximum size of the StageInfo's name.
 */
const int stage_name_max_size = 200;

/**
 * stage_env_max_size: defines the maximum size of the StageInfo's env.
 */
const int stage_env_max_size = 50;

/**
 * define HOST_NAME_MAX and LOGIN_NAME_MAX limits, for the StageInfo's hostname and login_name.
 */
#ifndef __USE_POSIX
#define HOST_NAME_MAX  64
#define LOGIN_NAME_MAX 64
#endif

/**
 * StageSimplifiedHandshakeRaw: Raw structure that identifies the Data Plane Stage.
 * - m_stage_name: defines the stage identifier (name).
 * - m_stage_env: defines the environment variable value registered for the data plane stage;
 * - m_pid: defines the pid of the process where the data plane stage is executing;
 * - m_ppid: defines the parent pid of the process where the data plane stage is executing.
 * - m_stage_hostname: defines the hostname of node that is executing the application.
 * - m_stage_user: defines the user that submitted the application.
 */
struct StageSimplifiedHandshakeRaw {
    char m_stage_name[stage_name_max_size] {};
    char m_stage_env[stage_env_max_size] {};
    int m_pid { -1 };
    int m_ppid { -1 };
    char m_stage_hostname[HOST_NAME_MAX] {};
    char m_stage_user[LOGIN_NAME_MAX] {};
};

/**
 * stage_max_handshake_address_size: defines the maximum size of the address to where the data plane
 * should connect after the handshake with the control plane.
 */
const int stage_max_handshake_address_size = 100;

/**
 * StageHandshakeRaw: Raw structure that identifies the address and port where the data plane should
 * connect after the handshake with the control plane.
 *  - m_address: defines the connection address;
 *  - m_port: defines the connection port. In the case of CommunicationType::unix, this value will
 *  not be used.
 */
struct StageHandshakeRaw {
    char m_address[stage_max_handshake_address_size] {};
    int m_port { -1 };
};

/**
 * HousekeepingOperation enum class.
 * Defines the supported types of housekeeping operations.
 * Currently, it supports two main operations for creating channels and enforcements objects in a
 * data plane stage. The remainder are expected to be improved as future work.
 *  - create_channel: Create a new Channel to receive I/O requests;
 *  - create_object: Create a new EnforcementObject, that respects to an existing Channel.
 *  - no_op: Default housekeeping rule type; does not correspond to any operation.
 *  Other types:
 *  - update: not implemented
 *  - remove: not implemented
 */
enum class HousekeepingOperation {
    create_channel = 1,
    create_object = 2,
    configure = 3,
    remove = 4,
    no_op = 0
};

/**
 * HousekeepingCreateChannelRaw: Raw structure to perform the serialization of HousekeepingRules of
 * type create_channel between the PAIO stage and the control plane.
 * - m_rule_id: defines the rule identifier;
 * - m_rule_type: defines the HousekeepingRule operation type (create_channel);
 * - m_channel_id: defines the Channel identifier;
 * - m_context_definition: defines the context definition used in this rule;
 * - m_workflow_id: defines the workflow identifier to be used for the classification and
 * differentiation of requests (including the creation of the differentiation token);
 * - m_operation_type: defines the operation type to be used for the classification and
 * differentiation of requests (including the creation of the differentiation token);
 * - m_operation_context: defines the operation context to be used for the classification and
 * differentiation of requests (including the creation of the differentiation token);
 */
struct HousekeepingCreateChannelRaw {
    uint64_t m_rule_id { 0 };
    int m_rule_type { static_cast<int> (HousekeepingOperation::create_channel) };
    long m_channel_id { -1 };
    int m_context_definition { static_cast<int> (ContextType::PAIO_GENERAL) };
    uint32_t m_workflow_id { 0 };
    uint32_t m_operation_type { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
    uint32_t m_operation_context { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
};

/**
 * HousekeepingCreateObjectRaw: Raw structure to perform the serialization of HousekeepingRules of
 * type create_object between the PAIO stage and the control plane.
 * - m_rule_id: defines the rule identifier;
 * - m_rule_type: defines the HousekeepingRule identifier (create_object);
 * - m_channel_id: defines the Channel identifier where the object should be created;
 * - m_enforcement_object_id: defines the EnforcementObject identifier;
 * - m_context_definition: defines the context definition used in this rule;
 * - m_operation_type: defines the operation type to be used for the classification and
 * differentiation of requests (including the creation of the differentiation token);
 * - m_operation_context: defines the operation context to be used for the classification and
 * differentiation of requests (including the creation of the differentiation token);
 * - m_enforcement_object_type: defines the type of the EnforcementObject to be created;
 * - m_property_first: defines the initial property (first) of the EnforcementObject;
 * - m_property_second: defines the initial property (second) of the EnforcementObject;
 */
struct HousekeepingCreateObjectRaw {
    long m_rule_id { 0 };
    int m_rule_type { static_cast<int> (HousekeepingOperation::create_object) };
    long m_channel_id { -1 };
    long m_enforcement_object_id { -1 };
    int m_context_definition { static_cast<int> (ContextType::PAIO_GENERAL) };
    uint32_t m_operation_type { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
    uint32_t m_operation_context { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
    long m_enforcement_object_type { 0 };
    long m_property_first { 0 };
    long m_property_second { 0 };
};

/**
 * EnforcementRuleRaw: Raw structure to perform the serialization of EnforcementRules between the
 * PAIO stage and control the plane.
 * - m_rule_id: defines the rule identifier;
 * - m_channel_id: defines the Channel identifier that contains the object to be enforced;
 * - m_enforcement_object_id: defines the EnforcementObject identifier to be enforced;
 * - m_enforcement_operation: defines the operation that should be enforced over the object (e.g.,
 * init, rate, ...);
 * - m_property_first: defines the property (first) to be set in the EnforcementObject;
 * - m_property_second: defines the property (second) to be set in the EnforcementObject;
 * - m_property_third: defines the property (third) to be set in the EnforcementObject;
 */
struct EnforcementRuleRaw {
    long m_rule_id { 0 };
    long m_channel_id { -1 };
    long m_enforcement_object_id { -1 };
    int m_enforcement_operation { 0 };
    long m_property_first { -1 };
    long m_property_second { -1 };
    long m_property_third { -1 };
};

/**
 * StageReadyRaw: Raw structure that defines if the data plane is ready to receive I/O requests
 * from the targeted I/O layer.
 */
struct StageReadyRaw {
    bool m_mark_stage { false };
};

/**
 * StatsGlobalRaw: Raw structure that holds the rate collected from a data plane stage.
 */
struct StatsGlobalRaw {
    double m_total_rate;
};

} // namespace cheferd

#endif // CHEFERD_INTERFACE_DEFINITIONS_HPP
