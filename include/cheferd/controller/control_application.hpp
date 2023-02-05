/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_CONTROL_APPLICATION_HPP
#define CHEFERD_CONTROL_APPLICATION_HPP

#include "cheferd/networking/stage_response/stage_response_ack.hpp"
#include "cheferd/networking/stage_response/stage_response_handshake.hpp"
#include "cheferd/networking/stage_response/stage_response_stat.hpp"

#include <array>
#include <atomic>
#include <cheferd/utils/logging.hpp>
#include <memory>

using namespace std::chrono;

namespace cheferd {

/**
 * Struct StageInfo.
 * Container that holds detailed information related to a data plane stage.
 */
struct StageInfo {
    std::string m_stage_name;
    std::string m_stage_env;
    std::string m_stage_user;
    std::string m_local_address;
};

/**
 * ControlApplication class.
 * The ControlApplication class serves as base class for the intelligence/control component of both
 * core and local control applications. It defines core variables and functions that are required
 * for the control component. Currently, the ControlApplication class contains the following
 * variables:
 * - m_feedback_loop_sleep_time: defines how long a feedback loop should take.
 * - working_application_: atomic value that marks if the control application is currently active.
 * - housekeeping_rules_ptr_: container used to store housekeeping rules that should be imposed at
 * the data plane stages.
 *
 */
class ControlApplication {

protected:
    const uint64_t m_feedback_loop_sleep_time;
    std::atomic<bool> working_application_;

    /**
     * sleep: Used to make control application main thread wait for the next loop.
     */
    virtual void sleep () = 0;

public:
    std::vector<std::string>* housekeeping_rules_ptr_;

    /**
     * ControlApplication default constructor.
     */
    ControlApplication () :
        m_feedback_loop_sleep_time { option_default_control_application_sleep },
        working_application_ { true },
        housekeeping_rules_ptr_ {}
    { }

    /**
     * ControlApplication default constructor.
     * @param rules_ptr
     */
    explicit ControlApplication (std::vector<std::string>* rules_ptr,
        const uint64_t& cycle_sleep_time) :
        m_feedback_loop_sleep_time { cycle_sleep_time },
        working_application_ { true },
        housekeeping_rules_ptr_ { rules_ptr }
    { }

    /**
     * ControlApplication parameterized constructor.
     * @param cycle_sleep_time Control feedback loop time.
     */
    explicit ControlApplication (const uint64_t& cycle_sleep_time) :
        m_feedback_loop_sleep_time { cycle_sleep_time },
        working_application_ { true },
        housekeeping_rules_ptr_ {}
    { }

    /**
     * ControlApplication default destructor.
     */
    virtual ~ControlApplication () = default;

    /**
     * operator: Used to initiate the control application feedback loop execution.
     */
    virtual void operator() () = 0;

    /**
     * stop_feedback_loop: Stops the feedback loop from executing.
     */
    virtual void stop_feedback_loop () = 0;
};

} // namespace cheferd

#endif // CHEFERD_CONTROL_APPLICATION_HPP
