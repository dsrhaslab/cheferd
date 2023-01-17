/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_INTERFACE_DEFINITIONS_HPP
#define CHEFERD_INTERFACE_DEFINITIONS_HPP

#include <cheferd/utils/context_propagation_definitions.hpp>
#include <climits>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace cheferd {

// TODO: remove these macros; define through an enum class
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

// TODO: remove these macros; instead of defines use the HousekeepingOperation enum class
// HousekeepingRules: create definitions
// #define HSK_CREATE_UNIT    1
#define HSK_CREATE_CHANNEL              1
#define HSK_CREATE_OBJECT               2
#define COLLECT_GLOBAL_STATS            5
#define COLLECT_GLOBAL_STATS_AGGREGATED 6

#define NO_OP 0

// #define HSK_ASSIGN 4
#define HSK_RASSIG 5 // probably remove this
#define HSK_REMOVE 6
#define HSK_UPDATE 7

// TODO: remove these macros; define through an enum class
// Enforcement tags (temporary)
#define ENF_INIT       1 // DRL object initialization
#define ENF_RATE       2 // set DRL rate
#define ENF_MAX_RATE   3 // set DRL maximum rate limit
#define ENF_REF_WINDOW 4 // set DRL refill window (period)
#define ENF_PREEMPT    5 // set DRL preempt operation
#define ENF_NONE       0

/**
 * Statistic collection metric. Define the type of statistic to collect, based
 * on our use cases (1 == RocksDB and 2 == TensorFlow).
 * TODO: remove; it should be more generic.
 */
#define ROCKSDB_STATISTIC_COLLECTION    1
#define TENSORFLOW_STATISTIC_COLLECTION 2

/**
 * Data plane instance.
 */
#define ROCKSDB_INSTANCE       0
#define TENSOR_FLOW_INSTANCE_1 1
#define TENSOR_FLOW_INSTANCE_2 2
#define TENSOR_FLOW_INSTANCE_3 3
#define TENSOR_FLOW_INSTANCE_4 4

typedef int dp_instance;

/**
 * ControlOperation structure.
 * Defines the metadata of the operation to be sent to the data plane stage.
 *  - m_operation_id: defines the control operation identifier;
 *  - m_operation_type: defines the type of operation to be received (housekeeping rule, enforcement
 *  rule, collect statistics, ...);
 *  - m_operation-subtype: defines the subtype of the operation to be received (create channel,
 *  create object, ...);
 *  - m_size: defines the size of the object to be received.
 * TODO: create dedicated class.
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
 * TODO: either remove it or update it.
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
 * - app_id: defines the user that submitted the application.
 * TODO: create dedicated object.
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
 * PAIO data plane stage. The remainder are expected to be improved as future work.
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
 * TODO: create dedicated class.
 */
struct HousekeepingCreateChannelRaw {
    uint64_t m_rule_id { 0 };
    int m_rule_type { static_cast<int> (HousekeepingOperation::create_channel) };
    long m_channel_id { -1 };
    int m_context_definition { static_cast<int> (ContextType::PAIO_GENERAL) };
    uint32_t m_workflow_id { 0 };
    uint32_t m_operation_type { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
    uint32_t m_operation_context { static_cast<uint32_t> (PAIO_GENERAL::no_op) };
    // TODO: remove differentiation/operation_context ...
    // int differentiation_context; // Channel's DifferentiationContext (FLOW, IO_OP, CUSTOM).
    // int operation_context; // Channel's OperationContext (BG_NO_OP, BG_FLUSH, ...).
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
 * TODO: create dedicated class; improve property definition (hardcoded); m_enforcement_object_type
 *  statically defined with EnforcementObject::Noop (problem with circular dependencies).
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
    // TODO: remove differentiation/operation_context ...
    // int differentiation_context; // Channel's DifferentiationContext (FLOW, IO_OP, CUSTOM).
    // int operation_context; // Channel's OperationContext (BG_NO_OP, BG_FLUSH, ...).
};

/**
 * ChannelDifferentiationClassifiersRaw: Raw structure that defines the which I/O classifiers
 * should be considered in the I/O classification and differentiation at Channels. The rule is
 * targeted for the overall data plane (i.e, all Channels).
 * - m_workflow_id: Boolean that defines if the workflow identifier I/O classifier should be
 * considered in I/O differentiation.
 * - m_operation_type: Boolean that defines if the operation type I/O classifier should be
 * considered in I/O differentiation.
 * - m_operation_context: Boolean that defines if the operation context I/O classifier should be
 * considered in I/O differentiation.
 * TODO: support in the SouthboundConnectionHandler; create dedicated object.
 */
struct ChannelDifferentiationClassifiersRaw {
    bool m_workflow_id { true };
    bool m_operation_type { false };
    bool m_operation_context { false };
};

/**
 * EnforcementObjectDifferentiationClassifiersRaw: Raw structure that defines the which I/O
 * classifiers should be considered in the I/O classification and differentiation at
 * EnforcementObjects. The rule is targeted for a specific Channel.
 * - m_channel_id: defines the identifier of the Channel that will assume these rules.
 * - m_operation_type: Boolean that defines if the operation type I/O classifier should be
 * considered in I/O differentiation.
 * - m_operation_context: Boolean that defines if the operation context I/O classifier should be
 * considered in I/O differentiation.
 * TODO: support in the SouthboundConnectionHandler; create dedicated object.
 */
struct EnforcementObjectDifferentiationClassifiersRaw {
    long m_channel_id { -1 };
    bool m_operation_type { false };
    bool m_operation_context { false };
};

/**
 * DifferentiationRuleRaw: Raw structure to perform the serialization of DifferentiationRules
 * between the PAIO stage and the control plane.
 * - m_rule_id: defines the rule identifier;
 * - m_rule_type: defines the type of the differentiation operation, namely if the rule respects to
 * the I/O differentiation at the channel or enforcement object;
 * - m_channel_id: defines the Channel identifier;
 * - m_enforcement_object_id: defines the EnforcementObject identifier;
 * - m_workflow_id: defines the workflow identifier classifier to perform the differentiation;
 * - m_operation_type: defines the operation type classifier to perform the differentiation;
 * - m_operation_context: defines the operation context classifier to perform the differentiation.
 * TODO: support in the SouthboundConnectionHandler; create dedicated object; m_rule_type
 *  statically defined with DifferentiationRuleType::none (problem with circular dependencies).
 */
struct DifferentiationRuleRaw {
    long m_rule_id { 0 };
    int m_rule_type { 0 }; // static_cast<int> (DifferentiationRuleType::none)
    long m_channel_id { -1 };
    long m_enforcement_object_id { -1 };
    uint32_t m_workflow_id { 0 };
    uint32_t m_operation_type { 0 };
    uint32_t m_operation_context { 0 };
};

/**
 * EnforcementRuleRaw: Raw structure to perform the serialization of EnforcementRules between the
 * PAIO stage and control the plane.
 * - m_rule_id: defines the rule identifier;
 * - m_channel_id: defines the Channel identifier that contains the object to be enforced;
 * - m_enforcement_object_id: defines the EnforcementObject identifier to be enforced;
 * - m_enforcement_object_type: defines the type of the EnforcementObject to be created;
 * - m_enforcement_operation: defines the operation that should be enforced over the object (e.g.,
 * init, rate, ...);
 * - m_property_first: defines the property (first) to be set in the EnforcementObject;
 * - m_property_second: defines the property (second) to be set in the EnforcementObject;
 * - m_property_third: defines the property (third) to be set in the EnforcementObject;
 * TODO: create dedicated class; improve property definition (hardcoded); m_enforcement_object_type
 *  statically defined with EnforcementObject::Noop (problem with circular dependencies);
 *  m_enforcement_operation statically defined as None.
 * Note: This is a very hardcoded struct: tailored for the DynamicRateLimiter object. Later, this
 *  will be update to serve any Object configuration. This features will need to be added and/or
 *  fixed in a later phase.
 */
struct EnforcementRuleRaw {
    long m_rule_id { 0 };
    long m_channel_id { -1 };
    long m_enforcement_object_id { -1 };
    // int m_enforcement_object_type { 0 }; // fixme: probably not needed; remove
    int m_enforcement_operation { 0 };
    long m_property_first { -1 };
    long m_property_second { -1 };
    long m_property_third { -1 };
};

/**
 * EnforcementRuleWithTimeRaw: Raw structure to perform the serialization of EnforcementRules
 * between the PAIO stage and control the plane.
 * - m_rule_time: Time beginning at zero that defines when a rule should be enforced (in seconds);
 * - m_rule_id: defines the rule identifier;
 * - m_channel_id: defines the Channel identifier that contains the object to be enforced;
 * - m_enforcement_object_id: defines the EnforcementObject identifier to be enforced;
 * - m_enforcement_object_type: defines the type of the EnforcementObject to be created;
 * - m_enforcement_operation: defines the operation that should be enforced over the object (e.g.,
 * init, rate, ...);
 * - m_property_first: defines the property (first) to be set in the EnforcementObject;
 * - m_property_second: defines the property (second) to be set in the EnforcementObject;
 * - m_property_third: defines the property (third) to be set in the EnforcementObject;
 * TODO: create dedicated class; improve property definition (hardcoded); m_enforcement_object_type
 *  statically defined with EnforcementObject::Noop (problem with circular dependencies);
 *  m_enforcement_operation statically defined as None.
 * Note: This is a very hardcoded struct: tailored for the DynamicRateLimiter object. Later, this
 *  will be update to serve any Object configuration. This features will need to be added and/or
 *  fixed in a later phase.
 */
struct EnforcementRuleWithTimeRaw {
    long m_rule_time { 0 };
    long m_rule_id { 0 };
    long m_channel_id { -1 };
    long m_enforcement_object_id { -1 };
    // int m_enforcement_object_type { 0 }; // fixme: probably not needed; remove
    int m_enforcement_operation { 0 };
    long m_property_first { -1 };
    long m_property_second { -1 };
    long m_property_third { -1 };
};

/**
 * StageReadyRaw: Raw structure that defines if the data plane is ready to receive I/O requests
 * from the targeted I/O layer.
 * TODO: complete me ...
 */
struct StageReadyRaw {
    bool m_mark_stage { false };
};

/**
 * StatsGlobalRaw: Raw structure to perform the serialization of I/O statistics between the
 * PAIO stage and the control plane. These statistics are specific to the "Per-application bandwidth
 * control" use case of the PAIO paper, where we demonstrate how to ensure QoS bandwidth across
 * multiple applications sharing the same storage device.
 * - m_fg_tasks:
 */

struct StatsGlobalRaw {
    double m_total_rate;
};

} // namespace cheferd

#endif // CHEFERD_INTERFACE_DEFINITIONS_HPP
