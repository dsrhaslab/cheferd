/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_OPTIONS_HPP
#define SHEPHERD_OPTIONS_HPP

#include <string>

namespace shepherd {

enum class CommunicationType { UNIX = 0, INET = 1, gRPC = 2 };

enum class ControllerType { CORE = 0, LOCAL = 1};

enum class ControlType { STATIC = 1, DYNAMIC = 2, MDS = 3, NOOP = 0 };

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
const std::string option_housekeeping_rules_file_path_
    = "/Users/ricardomacedo/Projects/sds-data-plane/shepherd/files/posix_housekeeping_rules_file";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";

const std::string option_housekeeping_rules_file_path_posix_static
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/posix_layer_housekeeping_rules_static_op";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";


const std::string option_housekeeping_rules_file_path_posix_dynamic
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/posix_layer_housekeeping_rules_dynamic";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";


const std::string option_housekeeping_rules_file_path_posix_mds
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/posix_layer_housekeeping_rules_mds";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";


const std::string option_housekeeping_rules_file_path_posix_metadata_data
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/posix_layer_housekeeping_rules_static_data_metadata";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";


const std::string option_housekeeping_rules_file_path_posix_total
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/posix_layer_housekeeping_rules_static_total";
//    = "home/acb11912na/db/shepherd/files/posix_housekeeping_rules_file";


/**
 * Default EnforcementRules file.
 * This parameter points to the path of the default rules to enforce at system
 * creation.
 */
const std::string option_differentiation_rules_file_path_
    = "/Users/ricardomacedo/Projects/sds-data-plane/shepherd/files/"
      "default_differentiation_rules_file";
//            "/home/gsd/db/shepherd/files/default_differentiation_rules_file";

/**
 * Default EnforcementRules file.
 * This parameter points to the path of the default rules to enforce at system
 * creation.
 */
const std::string option_enforcement_rules_file_path_
    = "/Users/ricardomacedo/Projects/sds-data-plane/shepherd/files/"
      "default_enforcement_rules_file_v2";
//            "/home/gsd/db/shepherd/files/default_enforcement_rules_file_v2";

/**
 * TensorFlow's HousekeepingRules file.
 * This parameter points to the path of the TensorFlow rules to insert and
 * enforce at system creation.
 */
const std::string option_tensorflow_housekeeping_rules_file_path_
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/tensorflow_housekeeping_rules_file";
        //"/Users/ricardomacedo/Projects/sds-data-plane/shepherd/files/"
      //"tensorflow_housekeeping_rules_file";
           // "/home/gsd/db/shepherd/files/tensorflow_housekeeping_rules_file";

/**
 * HousekeepingRules files.
 * This parameter points to the path of the rules to insert and
 * enforce at system creation.
 */
const std::string option_static_rules_with_time_file_path_job_op
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/static_rules_with_time_file_job_op";

const std::string option_static_rules_with_time_file_path_job_metadata
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/static_rules_with_time_file_job_metadata";

const std::string option_static_rules_with_time_file_path_job_data
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/static_rules_with_time_file_job_data";

const std::string option_static_rules_with_time_file_path_job
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/static_rules_with_time_file_job";

const std::string option_mds_rules_with_time_file_path_
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/mds_rules_with_time_file";

const std::string option_dynamic_rules_with_time_file_path_
    = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/files/dynamic_rules_with_time_file";

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
//ControllerType option_controller_ = ControllerType::CORE;

/**
 * Default SOCKET_NAME for UNIX Domain Socket based communications.
 */
const std::string option_socket_name_ = "/tmp/9Lq7BNBnBycd6nxy.socket";

/**
 * Default SOCKET_NAME for UNIX Domain Socket based communications -- TensorFlow
 * use case.
 */
const std::string option_socket_name_tf_1_ = "/tmp/paiotensorflow01.socket";
const std::string option_socket_name_tf_2_ = "/tmp/paiotensorflow02.socket";
const std::string option_socket_name_tf_3_ = "/tmp/paiotensorflow03.socket";
const std::string option_socket_name_tf_4_ = "/tmp/paiotensorflow04.socket";

/**
 * Default controller option.
 * This parameter is defined "a priori", as Sysadmins are not able to change it
 * at runtime.
 */
const bool option_is_core_controller_ = false;


const bool option_is_rocksdb_algorithm_ = false;

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

/**
 * Default MAX_BANDWIDTH_BPS.
 * This parameter defines the maximum available bandwidth for the TensorFlow's
 * control algorithm.
 */
const long option_max_bandwidth_bps = 1073741824;

/**
 * Default INSTANCE_BANDWIDTH_BPS.
 * This parameter defines the bandwidth demand for each TensorFlow instance
 * (data plane stage). Since our objective is to provide equity and fairness,
 * the instance bandwidth must be even for all stages.
 */
const long option_instance_bandwidth_bps = 262144000;

/**
 * Default calibrate token-buckets option.
 * This parameter defines if calibration of the token-buckets will be performed,
 * after assigning/computing the rate for each Data Plane stage.
 */
const bool option_calibrate_token_buckets = true;

} // namespace shepherd

#endif // SHEPHERD_OPTIONS_HPP
