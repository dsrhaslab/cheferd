/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

int result = 0;

void StopController (Controller controller)
{

    sleep (15);
    Logging::log_info ("cheferd controller terminating...");
    controller.StopController ();
    Logging::log_info ("cheferd controller terminated...");
}

void DeployController (ControllerType controller_type,
    ControlType control_type,
    std::string core_address,
    std::string local_address)
{
    Logging logger { cheferd::option_option_logging_ };
    Logging::log_info ("cheferd controller starting ...");

    switch (controller_type) {
        case ControllerType::CORE: {

            // create Controller
            Controller controller { control_type,
                core_address,
                option_default_control_application_sleep,
                1000000 };

            // create housekeeping rules files path list
            std::string housekeeping_rules_files_t {};

            switch (control_type) {
                case ControlType::STATIC: {
                    housekeeping_rules_files_t
                        = "../files/posix_layer_housekeeping_rules_static_test";
                }
                case ControlType::DYNAMIC_VANILLA: {
                    housekeeping_rules_files_t
                        = cheferd::option_housekeeping_rules_file_path_posix_total;
                    break;
                }
                case ControlType::DYNAMIC_LEFTOVER: {
                    housekeeping_rules_files_t
                        = cheferd::option_housekeeping_rules_file_path_posix_total;
                    break;
                }
                default:
                    result = 1;
                    break;
            }

            // create parser and read default housekeeping rules (global)
            controller.RegisterHousekeepingRules (housekeeping_rules_files_t);

            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlApplication ();

            controller.SpawnSystemAdmin ();

            // Stop controller after 15s
            std::thread xx (StopController, controller);
            xx.detach ();

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

            // Stop controller after 15s
            std::thread xx (StopController, controller);
            xx.detach ();

            // start connection manager to receive Data Plane Stage connections
            controller.SpawnConnectionManager ();
            break;
        }
        default: {
            result = 1;
            Logging::log_error ("Controller type not supported.");
            break;
        }
    }
}

int TestLocalCoreConnection ()
{

    std::string core_address = "0.0.0.0:50060";
    std::string local_address1 = "0.0.0.0:50062";
    std::string local_address2 = "0.0.0.0:50063";

    std::thread core_controller (&DeployController,
        ControllerType::CORE,
        ControlType::STATIC,
        core_address,
        "");
    sleep (2);

    std::thread local_controller1 (&DeployController,
        ControllerType::LOCAL,
        ControlType::STATIC,
        core_address,
        local_address1);
    sleep (2);

    std::thread local_controller2 (&DeployController,
        ControllerType::LOCAL,
        ControlType::STATIC,
        core_address,
        local_address2);

    core_controller.join ();
    local_controller1.join ();
    local_controller2.join ();

    return result;
}

} // namespace cheferd

int main ()
{
    return cheferd::TestLocalCoreConnection ();
}
