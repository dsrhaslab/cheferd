/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/utils/config_file_parser.hpp"

namespace cheferd {

// ConfigFileParser default constructor.
ConfigFileParser::ConfigFileParser ()
{
    Logging::log_debug ("ConfigFileParser default constructor.");
}

// ConfigFileParser default destructor.
ConfigFileParser::~ConfigFileParser ()
{
    Logging::log_debug ("ConfigFileParser default destructor.");
}

// select_default_housekeeping_rule call. Selects default housekeeping file if not defined
// in config file.
void ConfigFileParser::select_default_housekeeping_rule (ControlType control_type)
{

    switch (control_type) {
        case ControlType::STATIC: {
            housekeeping_rules_file = cheferd::option_housekeeping_rules_file_path_posix_total;
            break;
        }
        case ControlType::DYNAMIC_VANILLA: {
            housekeeping_rules_file = cheferd::option_housekeeping_rules_file_path_posix_dynamic;
            break;
        }
        case ControlType::DYNAMIC_LEFTOVER: {
            housekeeping_rules_file = cheferd::option_housekeeping_rules_file_path_posix_dynamic;
            break;
        }
        default:
            break;
    }
}

// select_control_type call. Selects control type from configuration file information.
void ConfigFileParser::select_control_type (YAML::Node root_node, int control)
{

    if (control == 1) {
        control_type = ControlType::STATIC;
    } else if (control == 2 || control == 3) {

        if (root_node["system_limit"]) {
            system_limit = root_node["system_limit"].as<long> ();
        } else {
            Logging::log_error ("System limit for control type needs  needs to be provided!");
        }

        if (control == 2) {
            control_type = ControlType::DYNAMIC_VANILLA;
        } else {
            control_type = ControlType::DYNAMIC_LEFTOVER;
        }
    } else if (control == 4) {
        control_type = ControlType::MDS;
    }
}

// process_core_controller_config call. Process core controller configuration.
void ConfigFileParser::process_core_controller_config (YAML::Node root_node)
{
    controller_type = ControllerType::CORE;

    if (root_node["core_address"]) {
        core_address = root_node["core_address"].as<std::string> ();
    } else {
        Logging::log_error ("Core controller address needs to be provided!");
    }

    if (root_node["control_type"]) {
        int control = root_node["control_type"].as<int> ();
        select_control_type (root_node, control);
    } else {
        control_type = ControlType::NOOP;
        Logging::log_info ("No control type chosen!");
    }

    if (root_node["housekeeping_rules_file"]) {
        housekeeping_rules_file = root_node["housekeeping_rules_file"].as<std::string> ();
    } else {
        select_default_housekeeping_rule (control_type);
        Logging::log_info ("Using default housekeeping rules!");
    }

    if (root_node["policies_rules_file"]) {
        policies_rules_file = root_node["policies_rules_file"].as<std::string> ();
    } else {
        Logging::log_error ("Policies rules file path needs to be provided!");
    }
}

// process_local_controller_config call. Process local controller configuration.
void ConfigFileParser::process_local_controller_config (YAML::Node root_node)
{
    controller_type = ControllerType::LOCAL;

    if (root_node["core_address"]) {
        core_address = root_node["core_address"].as<std::string> ();
    } else {
        Logging::log_error ("Core controller address needs to be provided!");
    }

    if (root_node["local_address"]) {
        local_address = root_node["local_address"].as<std::string> ();
    } else {
        Logging::log_error ("Local controller address needs to be provided!");
    }
}

// process_config_file call. Process configuration file.
void ConfigFileParser::process_config_file (const std::string& path)
{

    YAML::Node root_node = YAML::LoadFile (path);

    if (root_node["controller"]) {
        std::string controller = root_node["controller"].as<std::string> ();
        if (controller == "core") {
            process_core_controller_config (root_node);

        } else if (controller == "local") {
            process_local_controller_config (root_node);
        } else {
            Logging::log_error (
                "Controller in config option not supported (choose core or local)!");
        }
    }
}

} // namespace cheferd
