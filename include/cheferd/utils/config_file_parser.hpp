/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_CONFIG_FILE_PARSER_HPP
#define CHEFERD_CONFIG_FILE_PARSER_HPP

#include <cheferd/controller/controller.hpp>
#include <yaml-cpp/yaml.h>

namespace cheferd {

/**
 * ConfigFileParser class.
 * ConfigFileParser processes configuration file.
 * Currently, the ConfigFileParser class contains the following variables:
 * - controller_type: type of controller (e.g., CORE, LOCAL)
 * - control_type: type of control that control application imposes (e.g., STATIC,
 * DYNAMIC_VANILLA).
 * - core_address: core controller address.
 * - local_address: local controller address.
 * - housekeeping_rules_file: path to file that contains housekeeping rules.
 * - policies_rules_file:  path to file that contains policy rules.
 * - system_limit: defines the maximum limit of the system (e.g., IOPS, bandwidth).
 */
class ConfigFileParser {

public:
    ControllerType controller_type;
    ControlType control_type;
    std::string core_address;
    std::string local_address;
    std::string housekeeping_rules_file;
    std::string policies_rules_file;
    long system_limit;

    /**
     * process_config_file. Process configuration file.
     * @param path Path to configuration file.
     */
    void process_config_file (const std::string& path);

    /**
     * ConfigFileParser default constructor.
     */
    ConfigFileParser ();

    /**
     * ConfigFileParser default destructor.
     */
    ~ConfigFileParser ();

private:
    /**
     * select_default_housekeeping_rule: Selects default housekeeping file if not defined
     * in config file.
     * @param control_type Type of control (e.g., STATIC, DYNAMIC_VANILLA).
     */
    void select_default_housekeeping_rule (ControlType control_type);

    /**
     * select_control_type: Selects control type from configuration file information.
     * @param root_node YAML root node.
     * @param control Control type.
     */
    void select_control_type (YAML::Node root_node, int control);

    /**
     * process_core_controller_config. Process core controller configuration.
     * @param root_node YAML root node.
     */
    void process_core_controller_config (YAML::Node root_node);

    /**
     * process_local_controller_config: Process local controller configuration.
     * @param root_node YAML root node.
     */
    void process_local_controller_config (YAML::Node root_node);
};
}; // namespace cheferd

#endif // CHEFERD_CONFIG_FILE_PARSER_HPP
