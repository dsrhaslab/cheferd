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
 * Controller class.
 */
class Controller {

private:
    ControlApplication* m_control_application;

    ConnectionManager* m_connection_manager;

    SystemAdmin m_system_admin;

public:
    // public housekeeping_rules_ instance
    std::vector<std::string> m_housekeeping_rules;

public:
    /**
     * Controller default constructor.
     */
    // Controller ();

    /**
     * Controller parameterized constructor.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    // explicit Controller (const uint64_t& cycle_sleep_time);

    /**
     * Controller parameterized constructor.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    // Controller (const uint64_t& cycle_sleep_time, const std::string& connector_type);

    /**
     * Controller parameterized constructor.
     * @param controller_type Type of controller (Core or Local)
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    // Controller (ControllerType controller_type, const uint64_t& cycle_sleep_time);

    Controller (ControlType control_type,
        std::string& core_address,
        const uint64_t& cycle_sleep_time,
        long system_limit);

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
     * ...
     * @param path
     * @param total_rules
     */
    void RegisterHousekeepingRules (const std::string& paths);

    /*
     * SpawnControlAlgorithm call.
     */
    void SpawnControlAlgorithm ();

    /*
     * SpawnSystemAdmin call.
     */
    void SpawnSystemAdmin ();

    /**
     * SpawnConnectionManager call.
     */
    void SpawnConnectionManager ();

    /**
     * StopConnectionManager call.
     */
    void StopConnectionManager ();
};
} // namespace cheferd
#endif // CHEFERD_CONTROLLER_HPP
