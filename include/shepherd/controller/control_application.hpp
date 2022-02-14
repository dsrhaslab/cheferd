/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_CONTROL_APPLICATION_HPP
#define SHEPHERD_CONTROL_APPLICATION_HPP

#include <array>
#include <atomic>
#include <memory>
#include "shepherd/networking/stage_response/stage_response_ack.hpp"
#include "shepherd/networking/stage_response/stage_response_handshake.hpp"
#include "shepherd/networking/stage_response/stage_response_stats_entity.hpp"
#include "shepherd/networking/stage_response/stage_response_stats_global.hpp"

#include <shepherd/session/data_plane_session.hpp>
#include <shepherd/session/local_controller_session.hpp>
#include <shepherd/utils/logging.hpp>
//#include <shepherd/utils/make_unique.hpp>

using namespace std::chrono;

namespace shepherd {

class ControlApplication {

protected:
    std::atomic<int> m_active_local_controller_sessions; // this will be useful for use case 2
    std::atomic<int> m_pending_local_controller_sessions; // this will be useful for use case 2
    std::atomic<int> m_active_data_plane_sessions; // this will be useful for use case 2
    std::atomic<int> m_pending_data_plane_sessions; // this will be useful for use case 2

    const uint64_t m_feedback_loop_sleep_time;

    /**
     * DataPlaneSessionHandshake: (...)
     * @return
     */
    //virtual PStatus stage_handshake (int index) = 0;

    /**
     * CollectStatistics: (...)
     */
    virtual std::unique_ptr<StageResponse> collect_statistics () = 0;

    /**
     * Compute: (...)
     */
    virtual void compute (const std::unique_ptr<StageResponse>& statistics_ptr) = 0;

    /**
     * Sleep: (...)
     */
    virtual void sleep () = 0;

public:
    // Use the address as key,
    std::vector<std::string>* housekeeping_rules_ptr_;
    std::unordered_map<int, std::vector<std::string>> local_session_array_;
    std::unordered_map<std::string, int> local_address_to_index_;

    /**
     * ControlApplication default constructor.
     */
    ControlApplication () :
        m_active_local_controller_sessions { 0 },
        m_pending_local_controller_sessions { 0 },
        m_active_data_plane_sessions { 0 },
        m_pending_data_plane_sessions { 0 },
        m_feedback_loop_sleep_time { option_default_control_application_sleep },
        housekeeping_rules_ptr_ {},
        local_session_array_ {},
        local_address_to_index_{}
    { }

    /**
     * ControlApplication parameterized constructor.
     * @param rules_ptr
     */
    explicit ControlApplication (
        std::vector<std::string>* rules_ptr,
        const uint64_t& cycle_sleep_time) :
        m_active_local_controller_sessions { 0 },
        m_pending_local_controller_sessions { 0 },
        m_active_data_plane_sessions { 0 },
        m_pending_data_plane_sessions { 0 },
        m_feedback_loop_sleep_time { cycle_sleep_time },
        housekeeping_rules_ptr_ { rules_ptr },
        local_session_array_ {},
        local_address_to_index_{}
    { }

    /**
 * ControlApplication parameterized constructor.
 * @param rules_ptr
 */
    explicit ControlApplication (
        const uint64_t& cycle_sleep_time) :
        m_active_local_controller_sessions { 0 },
        m_pending_local_controller_sessions { 0 },
        m_active_data_plane_sessions { 0 },
        m_pending_data_plane_sessions { 0 },
        m_feedback_loop_sleep_time { cycle_sleep_time },
        housekeeping_rules_ptr_ {},
        local_session_array_ {},
        local_address_to_index_{}
    { }

    /**
     * ControlApplication default destructor.
     */
    virtual ~ControlApplication () = default;

    /**
     * Operator: (...)
     */
    virtual void operator() () = 0;

    /**
     * register_stage_session: (...)
     * @return
     */
    //virtual LocalControllerSession* register_local_controller_session (int index) = 0;

    //virtual DataPlaneSession* register_stage_session (int index) = 0;


};

} // namespace shepherd

#endif // SHEPHERD_CONTROL_APPLICATION_HPP
