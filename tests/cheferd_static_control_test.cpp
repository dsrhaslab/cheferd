/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

int collect_ok = 0;

void StopController (Controller controller)
{

    sleep (15);
    Logging::log_info ("cheferd controller terminating...");
    controller.StopConnectionManager ();
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
            Controller controller { ControllerType::CORE,
                control_type,
                core_address,
                option_default_control_application_sleep };

            // create housekeeping rules files path list
            std::string housekeeping_rules_files_t {};

            switch (control_type) {
                case ControlType::STATIC: {
                    housekeeping_rules_files_t
                        = cheferd::option_housekeeping_rules_file_path_posix_total;
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
            controller.SpawnControlAlgorithm ();

            // Stop controller after 15s
            std::thread xx (StopController, controller);
            xx.detach ();

            // start connection manager to receive LocalController connections
            controller.SpawnConnectionManager ();

            break;
        }
        case ControllerType::LOCAL: {
            // create Controller
            Controller controller { ControllerType::LOCAL,
                core_address,
                local_address,
                option_default_control_application_sleep };

            // spawn ControlAlgorithm to attach LocalControllerSessions and execute its
            // control algorithm ...
            controller.SpawnControlAlgorithm ();

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

void housekeeping_rule_channel (int sockfd)
{
    HousekeepingCreateChannelRaw object = {};

    int n = ::read (sockfd, &object, sizeof (struct HousekeepingCreateChannelRaw));

    printf ("Housekeeping Rule Channel: channel_%d:operation_type_%d : %d\n",
        object.m_channel_id,
        object.m_operation_type);

    ACK ack = {};
    ack.m_message = 1;

    n = ::write (sockfd, &ack, sizeof (struct ACK));
}

void housekeeping_rule_object (int sockfd)
{
    HousekeepingCreateObjectRaw object = {};

    int n = ::read (sockfd, &object, sizeof (struct HousekeepingCreateObjectRaw));

    printf ("Housekeeping Rule Object: channel_%d:enforcement_object_%d:operation_type_%d : %d\n",
        object.m_channel_id,
        object.m_enforcement_object_id,
        object.m_operation_type,
        object.m_property_first);

    ACK ack = {};
    ack.m_message = 1;

    n = ::write (sockfd, &ack, sizeof (struct ACK));
}

void mark_stage_ready (int sockfd)
{
    StageReadyRaw object = {};

    int n = ::read (sockfd, &object, sizeof (struct StageReadyRaw));

    printf ("Stage Ready\n");

    ACK ack = {};
    ack.m_message = 1;

    n = ::write (sockfd, &ack, sizeof (struct ACK));
}

void create_enforcement_rule (int sockfd)
{
    EnforcementRuleRaw object = {};

    int n = ::read (sockfd, &object, sizeof (struct EnforcementRuleRaw));

    printf ("Enforcement Rule: channel_%d:enforcement_object_%d : %d\n",
        object.m_channel_id,
        object.m_enforcement_object_id,
        object.m_property_first);

    ACK ack = {};
    ack.m_message = 1;

    n = ::write (sockfd, &ack, sizeof (struct ACK));
}

void collect_global_stats (int sockfd)
{
    printf ("DataPlaneStage  collect_global_stats\n");

    StatsGlobalRaw object = {};

    object.m_total_rate = -1;

    int n = ::write (sockfd, &object, sizeof (struct StatsGlobalRaw));
}

const int m_nr_entities = 2;

void collect_entity_stats (int sockfd)
{
    StatsEntityRaw object = {};

    printf ("DataPlaneStage  collect_entity_stats\n");

    for (int index = 0; index < m_nr_entities; index++) {
        object.stats[index] = 200 + index * 100;
    }

    int n = ::write (sockfd, &object, sizeof (struct StatsEntityRaw));
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

    ControlOperation operation1 = {};

    n = ::read (sockfd, &operation1, sizeof (struct ControlOperation));

    if (operation1.m_operation_type == 4) {
        if (operation1.m_operation_subtype == 1) {
            housekeeping_rule_channel (sockfd);
        } else if (operation1.m_operation_subtype == 2) {
            housekeeping_rule_object (sockfd);
        }
        Logging::log_info ("DeployDataPlaneStage: Housekeeping Rules Successful!");
    }

    ControlOperation operation2 = {};

    n = ::read (sockfd, &operation2, sizeof (struct ControlOperation));

    if (operation2.m_operation_type == 4) {
        if (operation2.m_operation_subtype == 1) {
            housekeeping_rule_channel (sockfd);
        } else if (operation2.m_operation_subtype == 2) {
            housekeeping_rule_object (sockfd);
        }
        Logging::log_info ("DeployDataPlaneStage: Housekeeping Rules Successful!");
    }

    ControlOperation operation3 = {};

    n = ::read (sockfd, &operation3, sizeof (struct ControlOperation));

    if (operation3.m_operation_type == 1) {
        mark_stage_ready (sockfd);
        Logging::log_info ("DeployDataPlaneStage: Mark stage ready!");
    }

    ControlOperation operation4 = {};

    n = ::read (sockfd, &operation4, sizeof (struct ControlOperation));

    if (operation4.m_operation_type == 3) {
        if (operation4.m_operation_subtype == 5) {
            collect_global_stats (sockfd);
            collect_ok = 1;
            Logging::log_info ("DeployDataPlaneStage: Collected Statistics!");
        }
    }
}

int TestCollectStatitics ()
{

    std::string core_address = "0.0.0.0:50051";
    std::string local_address1 = "0.0.0.0:50052";

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

    data_plane1.join ();

    Logging::log_info ("DeployDataPlaneStage: Exited!");

    core_controller.join ();

    Logging::log_info ("CoreController: Exited!");

    local_controller1.join ();

    Logging::log_info ("LocalController: Exited!");

    return collect_ok ? 0 : 1;
}

} // namespace cheferd

int main ()
{
    return cheferd::TestCollectStatitics ();
}
