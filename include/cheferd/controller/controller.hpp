/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
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
 * Provides the primitives to start the components: SystemAdmin,
 * ConnectionManager and ControlApplication;
 * And process the housekeeping rules.
 */
class Controller {

private:
    /**
     * Control application orchestrating the system.
     */
    ControlApplication* m_control_application;

    /**
     * Handles all connections made to the controller.
     */
    ConnectionManager* m_connection_manager;

    /**
     * Mimics the behavior of a system administrator.
     */
    SystemAdmin m_system_admin;

public:
    // public housekeeping_rules_ instance
    std::vector<std::string> m_housekeeping_rules;

    /**
     * Core Controller parameterized constructor.
     * @param control_type Type of control (Static, Dynamic_Vanilla, Dynamic_Leftover)
     * @param core_address Core controller address.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     * @param system_limit Maximum allowed operations in the system (IOPS or bandwidth)
     */
    Controller (ControlType control_type,
        std::string& core_address,
        const uint64_t& cycle_sleep_time,
        long system_limit);

    /**
     * Local Controller parameterized constructor.
     * @param core_address Core controller address.
     * @param local_address Local controller address.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    Controller (std::string& core_address,
        std::string& local_address,
        const uint64_t& cycle_sleep_time);

    /**
     * Controller default destructor.
     */
    ~Controller ();

    /**
     * RegisterHousekeepingRules call.
     * This can be later optimized. Used methods that were already implemented
     * @param path Path to the file that holds the housekeeping rules to be imposed.
     */
    void RegisterHousekeepingRules (const std::string& paths);

    /*
     * SpawnControlApplication call.
     * Starts the control application that orchestrates the system.
     */
    void SpawnControlApplication ();

    /*
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
