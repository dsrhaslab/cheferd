/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_CONTROLLER_HPP
#define CHEFERD_CONTROLLER_HPP

#include <cheferd/controller/core_control_application.hpp>
#include <cheferd/controller/local_control_application.hpp>
#include <cheferd/controller/system_admin.hpp>
#include <cheferd/networking/connection_manager.hpp>
#include <cheferd/networking/core_connection_manager.hpp>
#include <cheferd/networking/local_connection_manager.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/logging.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

/**
 * Controller class. Gateway component to start the whole system.
 * Provides the primitives to start the components: SystemAdmin, ConnectionManager
 * and ControlApplication and process inputted housekeeping rules.
 * Currently, the Controller class contains the following variables:
 * - m_control_application: defines control application orchestrating the system.
 * - m_connection_manager: handles all connections made to the controller.
 * - m_system_admin: mimics the behavior of a system administrator.
 * - m_housekeeping_rules: container used to store housekeeping rules that should be imposed at the
 * data plane stages.
 */
class Controller {

private:
    ControlApplication* m_control_application;
    ConnectionManager* m_connection_manager;
    SystemAdmin m_system_admin;

public:
    std::vector<std::string> m_housekeeping_rules;

    /**
     * Core Controller parameterized constructor.
     * @param control_type Type of control (STATIC, DYNAMIC_VANILLA, DYNAMIC_LEFTOVER).
     * @param core_address Core controller address.
     * @param cycle_sleep_time Amount of time that a feedback-loop cycle should take.
     * @param system_limit Maximum allowed operations in the system (e.g., IOPS or bandwidth)
     */
    Controller (ControlType control_type,
        std::string& core_address,
        const uint64_t& cycle_sleep_time,
        long system_limit);

    /**
     * Local Controller parameterized constructor.
     * @param core_address Core controller address.
     * @param local_address Local controller address.
     * @param cycle_sleep_time Amount of time that a feedback-loop cycle should take.
     */
    Controller (std::string& core_address,
        std::string& local_address,
        const uint64_t& cycle_sleep_time);

    /**
     * Controller default destructor.
     */
    ~Controller ();

    /**
     * RegisterHousekeepingRules call. Processes housekeeping rules.
     * @param path Path to the file that holds the housekeeping rules to be imposed.
     */
    void RegisterHousekeepingRules (const std::string& paths);

    /**
     * SpawnControlApplication call.
     * Starts the control application that orchestrates the system.
     */
    void SpawnControlApplication ();

    /**
     * SpawnSystemAdmin call.
     * Spawns a thread that mimics the behavior of a SysAdmin.
     */
    void SpawnSystemAdmin ();

    /**
     * SpawnConnectionManager call.
     * Spawn the connection manager to start to accept connections.
     */
    void SpawnConnectionManager ();

    /**
     * StopController call.
     * Stops the controller, including the connection manager
     * and control application.
     */
    void StopController ();
};
} // namespace cheferd
#endif // CHEFERD_CONTROLLER_HPP
