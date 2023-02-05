/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_OPTIONS_HPP
#define CHEFERD_OPTIONS_HPP

#include <string>

namespace cheferd {

enum class CommunicationType { UNIX = 0, INET = 1, gRPC = 2 };

enum class ControllerType { CORE = 0, LOCAL = 1 };

enum class ControlType { STATIC = 1, DYNAMIC_VANILLA = 2, DYNAMIC_LEFTOVER = 3, MDS = 4, NOOP = 0 };

enum class EnforcementChannelMode { SYNC = 0, ASYNC = 1 };

enum class EnforcementObjectType { DRL = 1, NOOP = 0 };

enum class DiffContext { FLOW = 0, IO_OP = 1, CUSTOM = 2 };

enum class OperationContext {
    BG_FLUSH = 1,
    BG_COMPACTION = 2,
    BG_CHECKPOINT = 3,
    BG_NO_OP = 4,
    FG_TASK = 0
};

/*
 * ****************************************************************
 * General Options: Default configurations of the data plane stage.
 * ****************************************************************
 */

/**
 * Default HousekeepingRules file.
 * This parameter points to the path of the default rules to insert and enforce
 * at system creation.
 */
const std::string option_housekeeping_rules_file_path_ = "../files/posix_housekeeping_rules_file";

const std::string option_housekeeping_rules_file_path_posix_static
    = "../files/posix_layer_housekeeping_rules_static_op";

const std::string option_housekeeping_rules_file_path_posix_dynamic
    = "../files/posix_layer_housekeeping_rules_dynamic";

const std::string option_housekeeping_rules_file_path_posix_metadata_data
    = "../files/posix_layer_housekeeping_rules_static_data_metadata";

const std::string option_housekeeping_rules_file_path_posix_total
    = "../files/posix_layer_housekeeping_rules_static_total";

/**
 * HousekeepingRules files.
 * This parameter points to the path of the rules to insert and
 * enforce at system creation.
 */
const std::string option_static_rules_with_time_file_path_job_op
    = "../files/static_rules_with_time_file_job_op";

const std::string option_static_rules_with_time_file_path_job_metadata
    = "../files/static_rules_with_time_file_job_metadata";

const std::string option_static_rules_with_time_file_path_job_data
    = "../files/static_rules_with_time_file_job_data";

const std::string option_static_rules_with_time_file_path_job
    = "../files/static_rules_with_time_file_job";

const std::string option_static_rules_with_time_file_path_user
    = "../files/static_rules_with_time_file_user";

const std::string option_dynamic_rules_with_time_file_path_
    = "../files/static_rules_with_time_file_job";

/**
 * Default communication option.
 * This parameter is defined "a priori", as Sysadmins are not able to change it
 * at runtime.
 */
const CommunicationType option_communication_ = CommunicationType::UNIX;

/**
 * Default controller option.
 * This parameter is defined "a priori", as Sysadmins are not able to change it
 * at runtime.
 */
const bool option_is_core_controller_ = false;

/**
 * Default BACKLOG for UNIX Domain Socket based communications.
 */
const int option_backlog_ = 10;

/**
 * Default PORT for TCP-based communications.
 */
const int option_port_ = 12345;

/**
 * Maximum connections with data plane stages.
 */
const size_t option_max_connections_ = 4;

/**
 * Default logging option.
 * This parameter defines if logging is enabled (true) or disabled (false).
 */
const bool option_option_logging_ = true;

/**
 * Default postpone enforcement rules.
 * This parameter postpones the enforcement of EnforcementRules (it is used in
 * the DataPlaneSession when dequeuing installed rules).
 */
const bool option_postpone_enforcement_rules_ = true;

/**
 * Default postpone enforcement rules time.
 * This parameter defines the time that EnforcementRules should be postponed.
 */
const long option_postpone_time_ = 2000;

/**
 * Default Control Application sleep time.
 * This parameter defines the amount of time that the control application sleeps
 * at each feedback-loop cycle.
 */
const uint64_t option_default_control_application_sleep = 1000000;

} // namespace cheferd

#endif // CHEFERD_OPTIONS_HPP
