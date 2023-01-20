/**
 *   Copyright (c) 2020 INESC TEC.
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

class ControlApplication {

protected:

    const uint64_t m_feedback_loop_sleep_time;

    std::atomic<bool> working_application_;

    /**
     * Sleep: (...)
     */
    virtual void sleep () = 0;

public:
    // Use the address as key,
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
     * ControlApplication parameterized constructor.
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
     * @param rules_ptr
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
     * Operator: (...)
     */
    virtual void operator() () = 0;
};

} // namespace cheferd

#endif // CHEFERD_CONTROL_APPLICATION_HPP
