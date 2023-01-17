//
//

#ifndef CHEFERD_CONFIG_FILE_PARSER_HPP
#define CHEFERD_CONFIG_FILE_PARSER_HPP

#include <cheferd/controller/controller.hpp>
#include <yaml-cpp/yaml.h>

namespace cheferd {

class ConfigFileParser {

public:
    ControllerType controller_type;
    ControlType control_type;
    std::string core_address;
    std::string local_address;
    std::string housekeeping_rules_file;
    std::string policies_rules_file;
    long system_limit;

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
    void select_default_housekeeping_rule (ControlType control_type);
    void select_control_type (YAML::Node root_node, int control);
    void process_core_controller_config (YAML::Node root_node);
    void process_local_controller_config (YAML::Node root_node);
};
}; // namespace cheferd

#endif // CHEFERD_CONFIG_FILE_PARSER_HPP
