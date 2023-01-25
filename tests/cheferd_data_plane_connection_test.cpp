/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

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
            Logging::log_error ("Controller type not supported.");
            break;
        }
    }
}

void DeployDataPlaneStage (char* stage_name,
    char* stage_env,
    char* stage_user,
    int m_pid,
    int m_ppid,
    char* socket_name)
{
    int sockfd, n;
    struct sockaddr_un serv_addr;

    char buffer[256];

    sockfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        Logging::log_error ("DeployDataPlaneStage: Error in Opening Socket 1!");
    }

    StageSimplifiedHandshakeRaw object = {};

    strcpy (object.m_stage_name, stage_name);
    strcpy (object.m_stage_env, stage_env);
    strcpy (object.m_stage_user, stage_user);

    object.m_pid = m_pid;
    object.m_ppid = m_ppid;

    const char* option_socket_name = socket_name;

    bzero ((char*)&serv_addr, sizeof (serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy (serv_addr.sun_path, option_socket_name, sizeof (serv_addr.sun_path) - 1);

    if (connect (sockfd, (struct sockaddr*)&serv_addr, sizeof (serv_addr)) < 0) {
        Logging::log_error ("DeployDataPlaneStage: Error in Connection 1!");
    }

    bzero (buffer, 256);

    ControlOperation operation = {};

    n = ::read (sockfd, &operation, sizeof (struct ControlOperation));

    Logging::log_info (
        "DeployDataPlaneStage: Here is message 1:" + std::to_string (operation.m_operation_type));

    n = ::write (sockfd, &object, sizeof (struct StageSimplifiedHandshakeRaw));

    if (n < 0) {
        Logging::log_error ("DeployDataPlaneStage: Error wrinting to Socket!");
    }

    StageHandshakeRaw handshake_object = {};

    n = ::read (sockfd, &handshake_object, sizeof (struct StageHandshakeRaw));

    Logging::log_info (
        "DeployDataPlaneStage: Here is message 2:" + std::to_string (operation.m_operation_type));

    const char* option_socket_name2 = handshake_object.m_address;

    close (sockfd);
    sockfd = socket (AF_UNIX, SOCK_STREAM, 0);

    bzero ((char*)&serv_addr, sizeof (serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy (serv_addr.sun_path, option_socket_name2, sizeof (serv_addr.sun_path) - 1);

    if (connect (sockfd, (struct sockaddr*)&serv_addr, sizeof (serv_addr)) < 0) {
        Logging::log_error ("DeployDataPlaneStage: Error in Connection 2!");
    } else {
        Logging::log_info ("DeployDataPlaneStage: Connection Successful!");
    }
}

void TestDataPlaneConnection ()
{

    std::string core_address = "0.0.0.0:50051";
    std::string local_address1 = "0.0.0.0:50052";

    /*std::thread core_controller (&DeployController,
                                 ControllerType::CORE,
                                 ControlType::STATIC,
                                 core_address,
                                 "");*/
    sleep (2);

    std::thread local_controller1 (&DeployController,
        ControllerType::LOCAL,
        ControlType::STATIC,
        core_address,
        local_address1);
    sleep (2);

    char* socket_name = "/tmp/0.0.0.0:50052.socket";

    char* stage_name1 = "tensor";
    char* stage_env1 = "1";
    char* stage_user1 = "user1";

    std::thread data_plane1 (&DeployDataPlaneStage,
        stage_name1,
        stage_env1,
        stage_user1,
        100,
        101,
        socket_name);

    char* stage_name2 = "tensor";
    char* stage_env2 = "2";
    char* stage_user2 = "user1";

    std::thread data_plane2 (&DeployDataPlaneStage,
        stage_name2,
        stage_env2,
        stage_user2,
        200,
        201,
        socket_name);

    char* stage_name3 = "kvs";
    char* stage_env3 = "1";
    char* stage_user3 = "user2";

    std::thread data_plane3 (&DeployDataPlaneStage,
        stage_name3,
        stage_env3,
        stage_user3,
        300,
        301,
        socket_name);

    data_plane1.join ();
    data_plane2.join ();
    data_plane3.join ();

    local_controller1.join ();

    // core_controller.join();
}

} // namespace cheferd

int main ()
{
    cheferd::TestDataPlaneConnection ();
    return 0;
}
