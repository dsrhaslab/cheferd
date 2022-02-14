/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/controller/controller.hpp>
//#include <shepherd/controller/local_controller.hpp>
#include <shepherd/networking/connection_manager.hpp>
#include <shepherd/session/policy_generator.hpp>
#include <shepherd/utils/logging.hpp>

using namespace shepherd;


#include <boost/program_options.hpp>
namespace po = boost::program_options;

void process_program_options (int argc, char** argv, ControllerType& controller_type, ControlType& control_type, std::string& core_address, std::string& local_address)
{
    po::options_description description("Control Plane Controller Usage");

    std::string address1;
    std::string address2;
    int controller;
    int control;

    //("version, v", "Display the version number")
    //("communication, cm", "Define type of communication (UNIX, INET, gRPC)");

    description.add_options()
        ("help", "Display this help message")
        ("controller", po::value<int>(&controller)->default_value(0), "Define type of controller (0 CORE or 1 LOCAL)")
        ("core_address", po::value<std::string>(&address1)->required(),"Address for the core controller.")
        ("local_address", po::value<std::string>(&address2),"Address for the local controller.")
        ("control",  po::value<int>(&control)->default_value(0), "Define type of control type (1 STATIC or 2 DYNAMIC or 3 MDS or 0 NO_CONTROL).")
        ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc,argv).options(description).run(), vm);
    po::notify(vm);

    if (vm.count("help")){
        std::cout << description << "\n";
    }

    static std::regex address_regex("(\\d{1,3}.){3}\\d{1,3}:\\d+");

    if (vm.count("controller")){
        if (controller == 0){
            controller_type = ControllerType::CORE;

            if (control == 1){
                control_type = ControlType::STATIC;
            }
            else if (control == 2){
                control_type = ControlType::DYNAMIC;
            }
            else if (control == 3){
                control_type = ControlType::MDS;
            }
            else {
                control_type = ControlType::NOOP;
            }
        }
        else if (controller == 1){
            controller_type = ControllerType::LOCAL;

            if (vm.count("local_address")) {
                if (std::regex_match (address1, address_regex)) {
                    local_address = address2;
                } else {
                    throw po::validation_error (po::validation_error::invalid_option_value);
                }
            }
        }
        else {
            //TODO: Put with validation_error
            /*throw po::validation_error(
                po::validation_error::invalid_option_value,
                "controller",
                vm["controller"]
            );*/
            Logging::log_error ("Option of Controller type not supported. Choose 0 or 1 (0-CORE or 1-LOCAL)");
        }
    }


    if (vm.count("core_address")) {
        if (std::regex_match (address1, address_regex)) {
            core_address = address1;
        } else {
            throw po::validation_error (po::validation_error::invalid_option_value);
        }
    }

}



int main (int argc, char** argv)
{
    Logging logger { shepherd::option_option_logging_ };
    Logging::log_info ("Shepherd controller starting ...");


    ControllerType controller_type;
    ControlType control_type;
    std::string core_address;
    std::string local_address;

    // Parse and process the command line
    process_program_options (argc, argv, controller_type, control_type, core_address, local_address);

    //std::cout << "host:\t"   << std::to_string(controller_type)      << "\n";



    //= (option_is_core_controller_ ? ControllerType::CORE : ControllerType::LOCAL);

    switch ( controller_type ) {
        case ControllerType::CORE: {

            // create Controller
            Controller controller { ControllerType::CORE, control_type, core_address, option_default_control_application_sleep };

            // create housekeeping rules files path list
            std::string housekeeping_rules_files_t {};

            switch (control_type) {
                case ControlType::STATIC: {
                    housekeeping_rules_files_t = shepherd::option_housekeeping_rules_file_path_posix_total;
                }
                case ControlType::DYNAMIC: {
                    // 1st phase: collect statistics from existing data plane stage

                    break;
                }
                case ControlType::MDS: {
                    housekeeping_rules_files_t = shepherd::option_housekeeping_rules_file_path_posix_mds;
                }
                default:
                    break;
            }

            // create parser and read default housekeeping rules (global)
            controller.RegisterHousekeepingRules (housekeeping_rules_files_t);

            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlAlgorithm ("global");

            controller.SpawnSystemAdmin ();

            // start connection manager to receive LocalController connections
            controller.Start ("global");

            break;
        }
        case ControllerType::LOCAL: {
            // create Controller
            Controller controller { ControllerType::LOCAL, core_address, local_address, option_default_control_application_sleep };


            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlAlgorithm ("local");

            // start connection manager to receive LocalController connections
            controller.Start ("local");
            break;
        }
        default: {
            Logging::log_error ("Controller type not supported.");
            break;
        }
    }



    return 0;
}
