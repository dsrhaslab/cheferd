/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_CONTROLLER_HPP
#define SHEPHERD_CONTROLLER_HPP

#include <shepherd/controller/core_control_application.hpp>
#include <shepherd/controller/local_control_application.hpp>
#include <shepherd/controller/system_admin.hpp>
#include <shepherd/networking/connection_manager.hpp>
#include <shepherd/session/policy_generator.hpp>
#include <shepherd/utils/logging.hpp>
#include <shepherd/utils/rules_file_parser.hpp>

namespace shepherd {

/**
 * Controller class.
 */
class Controller {

private:
    ControlApplication* m_control_application;

    ConnectionManager m_connection_manager;

    SystemAdmin m_system_admin;

public:
    // public housekeeping_rules_ instance
    std::vector<std::string> m_housekeeping_rules;




public:
    /**
     * Controller default constructor.
     */
    //Controller ();

    /**
     * Controller parameterized constructor.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    //explicit Controller (const uint64_t& cycle_sleep_time);

    /**
     * Controller parameterized constructor.
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    //Controller (const uint64_t& cycle_sleep_time, const std::string& connector_type);

    /**
     * Controller parameterized constructor.
     * @param controller_type Type of controller (Core or Local)
     * @param cycle_sleep_time Amount of time that the control application
     * sleeps at each feedback-loop cycle.
     */
    //Controller (ControllerType controller_type, const uint64_t& cycle_sleep_time);


    Controller (ControllerType controller_type,
                ControlType control_type,
                std::string& core_address,
                const uint64_t& cycle_sleep_time);

    Controller (ControllerType controller_type,
                            std::string& core_address,
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
    void RegisterHousekeepingRules (const std::string paths);

    /*
     * SpawnControlAlgorithm call.
     * TODO: refactor -- the method should simply spawn the control algorithm
     * from the base class, and not differentiate
     */
    void SpawnControlAlgorithm (const std::string& controller_type);

    void SpawnSystemAdmin ();

    /**
     * Start call.
     * TODO: refactor -- the method should simply spawn the control algorithm
     * from the base class, and not differentiate
     */
    void Start (const std::string& controller_type);
};
} // namespace shepherd
#endif // SHEPHERD_CONTROLLER_HPP
