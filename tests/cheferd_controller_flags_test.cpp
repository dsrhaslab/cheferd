/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/utils/command_line_parser.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

int TestCoreControllerFlags ()
{

    int argc = 7;
    char* argv[7]
        = { "./cheferd", "--controller", "0", "--core_address", "0.0.0.0:50051", "--control", "1" };

    CommandLineParser commandLineParser;

    // Parse and process the command line
    commandLineParser.process_program_options (argc, argv);

    int return_value = 0;

    ControllerType controller_type = commandLineParser.controller_type;
    ControlType control_type = commandLineParser.control_type;
    std::string core_address = commandLineParser.core_address;
    std::string local_address = commandLineParser.local_address;

    Logging::log_info ("Info Submitted: ");

    switch (controller_type) {
        case ControllerType::CORE: {
            Logging::log_info ("Core Controller, ");
            switch (control_type) {
                case ControlType::STATIC: {
                    Logging::log_info ("Static Control, ");
                    break;
                }
                case ControlType::DYNAMIC_VANILLA: {
                    Logging::log_info ("Dynamic Control Vanilla, ");
                    break;
                }
                case ControlType::DYNAMIC_LEFTOVER: {
                    Logging::log_info ("Dynamic Control Leftover, ");
                    break;
                }
                default:
                    Logging::log_info ("Control Type incorrect, ");
                    return_value = 1;
                    break;
            }
            break;
        }
        default:
            Logging::log_info ("Controller Type incorrect, ");
            return_value = 1;
            break;
    }

    Logging::log_info ("core_address: " + core_address + ".");

    return return_value;
}

int TestLocalControllerFlags ()
{
    int argc = 7;
    char* argv[7] = { "./cheferd",
        "--controller",
        "1",
        "--local_address",
        "0.0.0.0:50052",
        "--core_address",
        "0.0.0.0:50051" };

    CommandLineParser commandLineParser;

    // Parse and process the command line
    commandLineParser.process_program_options (argc, argv);

    int return_value = 0;

    ControllerType controller_type = commandLineParser.controller_type;
    ControlType control_type = commandLineParser.control_type;
    std::string core_address = commandLineParser.core_address;
    std::string local_address = commandLineParser.local_address;

    Logging::log_info ("Info Submitted: ");

    switch (controller_type) {
        case ControllerType::LOCAL: {
            Logging::log_info ("Local Controller, ");
            break;
        }
        default:
            Logging::log_info ("Controller Type incorrect, ");
            return_value = 1;
            break;
    }

    Logging::log_info ("local_address: " + local_address + ", ");
    Logging::log_info ("core_address: " + core_address + ".");

    return return_value;
}

} // namespace cheferd

int main ()
{
    int res1 = cheferd::TestCoreControllerFlags ();
    int res2 = cheferd::TestLocalControllerFlags ();
    return res1 || res2;
}
