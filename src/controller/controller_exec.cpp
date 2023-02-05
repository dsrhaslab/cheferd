/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/command_line_parser.hpp>
#include <cheferd/utils/config_file_parser.hpp>
#include <cheferd/utils/logging.hpp>

using namespace cheferd;

int main (int argc, char** argv)
{
    Logging logger { cheferd::option_option_logging_ };
    Logging::log_info ("cheferd controller starting ...");

    CommandLineParser commandLineParser;

    // Parse and process the command line
    commandLineParser.process_program_options (argc, argv);
    std::string config_file_path = commandLineParser.config_file_path;

    // Parse and process config file
    ConfigFileParser configFileParser;
    configFileParser.process_config_file (config_file_path);

    ControllerType controller_type = configFileParser.controller_type;
    ControlType control_type = configFileParser.control_type;
    std::string core_address = configFileParser.core_address;
    std::string local_address = configFileParser.local_address;
    std::string housekeeping_rules_file = configFileParser.housekeeping_rules_file;
    std::string policies_rules_file = configFileParser.policies_rules_file;
    long system_limit = configFileParser.system_limit;

    switch (controller_type) {
        case ControllerType::CORE: {

            // create Controller
            Controller controller { control_type,
                core_address,
                option_default_control_application_sleep,
                system_limit };

            // create housekeeping rules files path list
            std::string housekeeping_rules_files_t {};

            housekeeping_rules_files_t = housekeeping_rules_file;

            // create parser and read default housekeeping rules (global)
            controller.RegisterHousekeepingRules (housekeeping_rules_files_t);

            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlApplication ();

            controller.SpawnSystemAdmin ();

            // start connection manager to receive LocalController connections
            controller.SpawnConnectionManager ();

            break;
        }
        case ControllerType::LOCAL: {
            // create Controller
            Controller controller { core_address,
                local_address,
                option_default_control_application_sleep };

            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlApplication ();

            // start connection manager to receive Data Plane Stage connections
            controller.SpawnConnectionManager ();
            break;
        }
        default: {
            Logging::log_error ("Controller type not supported.");
            break;
        }
    }

    return 0;
}
