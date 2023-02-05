/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/core_control_application.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
}

namespace cheferd {

// CoreControlApplication parameterized constructor.
CoreControlApplication::CoreControlApplication (ControlType control_type,
    std::vector<std::string>* rules_ptr,
    const uint64_t& cycle_sleep_time,
    long system_limit) :
    ControlApplication { rules_ptr, cycle_sleep_time },
    change_in_system { false },
    local_queue {},
    local_to_data_queue_ {},
    m_control_type { control_type },
    local_sessions_ {},
    local_to_stages {},
    stage_info_detailed {},
    job_location_tracker {},
    job_demands {},
    job_rates {},
    job_previous_rates {},
    maximum_limit { system_limit },
    active_ops {},
    m_active_local_controller_sessions { 0 },
    m_pending_local_controller_sessions { 0 },
    m_active_data_plane_sessions { 0 },
    m_pending_data_plane_sessions { 0 }
{
    Logging::log_info ("CoreControlApplication parameterized constructor.");
}

// CoreControlApplication default destructor.
CoreControlApplication::~CoreControlApplication ()
{
    Logging::log_info ("CoreControlApplication: exiting ...\n");
}

////////////////////////////////////////////
///////// Register new sessions ////////////
////////////////////////////////////////////

// register_local_controller_session call. Register a new local controller.
void CoreControlApplication::register_local_controller_session (
    const std::string& local_controller_address)
{
    Logging::log_debug ("RegisterLocalControllerSession -- " + local_controller_address);

    pending_register_session_lock_.lock ();
    local_queue.emplace (local_controller_address);
    pending_register_session_lock_.unlock ();

    this->m_pending_local_controller_sessions.fetch_add (1);
}

// register_stage_session call. Register a new data plane.
void CoreControlApplication::register_stage_session (const std::string& local_controller_address,
    const std::string& stage_name,
    const std::string& stage_env,
    const std::string& stage_user)
{
    Logging::log_debug ("RegisterDataPlaneSession --" + local_controller_address + " : "
        + stage_name + "+" + stage_env);

    std::unique_ptr<StageInfo> all_stage_info = std::make_unique<StageInfo> ();
    all_stage_info->m_stage_name = stage_name;
    all_stage_info->m_stage_env = stage_env;
    all_stage_info->m_stage_user = stage_user;
    all_stage_info->m_local_address = local_controller_address;

    pending_register_stage_lock_.lock ();

    local_to_data_queue_.emplace (std::move (all_stage_info));
    pending_register_stage_lock_.unlock ();

    this->m_pending_data_plane_sessions.fetch_add (1);
}

////////////////////////////////////////////
/////////////// Feedback Loop //////////////
////////////////////////////////////////////

// operator call. Used to initiate the control application feedback loop execution.
void CoreControlApplication::operator() ()
{
    this->execute_feedback_loop ();
}

// stop_feedback_loop call. Stops the feedback loop from executing.
void CoreControlApplication::stop_feedback_loop ()
{
    working_application_ = false;
    for (auto const& local_session : local_sessions_) {
        local_session.second->RemoveSession ();
    }
}

// execute_feedback_loop call. Executes feedback loop.
void CoreControlApplication::execute_feedback_loop ()
{
    initialize ();

    Logging::log_debug ("CoreControlApplication::ExecuteFeedbackLoop");
    PStatus status = PStatus::Error ();
    working_application_ = true;

    while (working_application_.load () && this->m_pending_local_controller_sessions.load () == 0) {
        std::this_thread::sleep_for (milliseconds (100));
    }

    int rounds_counter = 0;
    std::list<std::string> session_temp = {};
    std::list<std::string>& sessions_sent = session_temp;

    while (working_application_.load ()
        && (this->m_pending_local_controller_sessions.load ()
               + this->m_active_local_controller_sessions.load ()
               + this->m_pending_data_plane_sessions.load ())
            > 0) {

        auto start = std::chrono::steady_clock::now ();

        while (this->m_pending_local_controller_sessions.load () > 0) {
            handle_local_controller_sessions ();
        }

        while (this->m_pending_data_plane_sessions.load () > 0) {
            handle_data_plane_sessions ();
        }

        switch (m_control_type) {
            case ControlType::STATIC: {
                const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats
                    = this->collect_statistics_global ();

                auto end = std::chrono::steady_clock::now ();
                Logging::log_debug ("Latency Collect= "
                    + std::to_string (
                        std::chrono::duration_cast<std::chrono::microseconds> (end - start)
                            .count ())
                    + "[µs]");

                if (this->m_active_local_controller_sessions.load () > 0)
                    this->compute_and_enforce_static_rules (d_stats);

                break;
            }
            case ControlType::DYNAMIC_VANILLA: {
                const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats
                    = this->collect_statistics_global ();

                this->compute_and_enforce_dynamic_vanilla_rules (d_stats);
                break;
            }
            case ControlType::DYNAMIC_LEFTOVER: {
                if (rounds_counter == 0) {
                    sessions_sent = this->collect_statistics_global_send ();
                }

                rounds_counter++;
                if (rounds_counter == COLLECT_ROUNDS) {
                    const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats
                        = this->collect_statistics_global_collect (sessions_sent);

                    this->compute_and_enforce_dynamic_leftover_rules (d_stats, sessions_sent);
                    rounds_counter = 0;
                }
                break;
            }

            default:
                break;
        }

        change_in_system = false;

        auto end = std::chrono::steady_clock::now ();
        // 3rd phase: sleep for the next feedback loop cycle
        Logging::log_debug ("Latency Round= "
            + std::to_string (
                std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ())
            + "[µs]");

        std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time
            - std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ()));
    }

    // log message and end control loop
    Logging::log_info ("Exiting. No active connections.");
    _exit (EXIT_SUCCESS);
}

////////////////////////////////////////////
//////////// Collect Statistics ////////////
////////////////////////////////////////////

// collect_statistics_global_send call. Sends to local controllers the request to collect
// statistics.
std::list<std::string> CoreControlApplication::collect_statistics_global_send ()
{
    Logging::log_debug ("ControlApplication:collect_statistics_global_send");

    std::list<std::string> sessions_sent {};

    // create COLLECT_GLOBAL_STATS request
    std::string rule = std::to_string (COLLECT_DETAILED_STATS) + "|"
        + std::to_string (COLLECT_GLOBAL_STATS_AGGREGATED) + "|";

    // submit requests to each LocalControllerSession's submission_queue
    for (auto const& local_session : local_sessions_) {
        // put request on LocalControllerSession::submission_queue
        if (local_to_stages.find (local_session.first) != local_to_stages.end ()
            && !local_to_stages.at (local_session.first).empty ()) {
            local_session.second->SubmitRule (rule);
            sessions_sent.push_back (local_session.first);
        }
    }

    return sessions_sent;
}

// collect_statistics_global_collect call. Collects statistics from data plane stages.
std::unordered_map<std::string, std::unique_ptr<StageResponse>>
CoreControlApplication::collect_statistics_global_collect (std::list<std::string>& sessions_sent)
{
    Logging::log_debug ("ControlApplication:collect_statistics_global_collect");

    std::unordered_map<std::string, std::unique_ptr<StageResponse>> collected_stats {};
    std::list<std::string> sessions_to_delete;

    // collect requests from each DataPlaneSession's completion_queue
    for (auto const& local_session : local_sessions_) {
        std::string local_address = local_session.first;

        // wait for request to be on DataPlaneSession::completion_queue
        if (std::find (sessions_sent.begin (), sessions_sent.end (), local_address)
                != sessions_sent.end ()
            && local_to_stages.find (local_address) != local_to_stages.end ()
            && !local_to_stages.at (local_address).empty ()) {
            collect_statistics_result (local_session.second,
                local_address,
                sessions_to_delete,
                collected_stats);
        }
    }

    for (auto const& local_session : sessions_to_delete) {
        Logging::log_error (
            "Collect_statistics_global: Connection error; disconnecting from instance-"
            + local_session);
        this->m_active_local_controller_sessions.fetch_sub (1);
        local_sessions_.erase (local_session);
    }

    return collected_stats;
}

// collect_statistics_global call. Sends the request and collects statistics from data plane stages.
std::unordered_map<std::string, std::unique_ptr<StageResponse>>
CoreControlApplication::collect_statistics_global ()
{
    Logging::log_debug ("ControlApplication:collect_statistics_global");

    std::unordered_map<std::string, std::unique_ptr<StageResponse>> collected_stats {};

    // create COLLECT_GLOBAL_STATS request
    std::string rule = std::to_string (COLLECT_DETAILED_STATS) + "|"
        + std::to_string (COLLECT_GLOBAL_STATS) + "|";

    // submit requests to each LocalControllerSession's submission_queue
    for (auto const& local_session : local_sessions_) {
        // put request on LocalControllerSession::submission_queue
        if (local_to_stages.find (local_session.first) != local_to_stages.end ()
            && !local_to_stages.at (local_session.first).empty ()) {
            local_session.second->SubmitRule (rule);
        }
    }

    std::list<std::string> sessions_to_delete;

    // collect requests from each DataPlaneSession's completion_queue
    for (auto const& local_session : local_sessions_) {
        std::string local_address = local_session.first;

        // wait for request to be on DataPlaneSession::completion_queue
        if (local_to_stages.find (local_address) != local_to_stages.end ()
            && !local_to_stages.at (local_address).empty ()) {
            collect_statistics_result (local_session.second,
                local_address,
                sessions_to_delete,
                collected_stats);
        }
    }

    for (auto const& local_session : sessions_to_delete) {
        Logging::log_error (
            "collect_statistics_global2: Connection error; disconnecting from instance-"
            + local_session);
        this->m_active_local_controller_sessions.fetch_sub (1);
        local_sessions_.erase (local_session);
    }

    return collected_stats;
}

////////////////////////////////////////////
//////////////// Sleep /////////////////////
////////////////////////////////////////////

// sleep call. Used to make control application main thread wait for the next loop.
void CoreControlApplication::sleep ()
{
    std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time));
}

////////////////////////////////////////////
////// Compute and Enforce new rules ///////
////////////////////////////////////////////

// compute_and_enforce_static_rules call. Computes and enforces STATIC policies.
void CoreControlApplication::compute_and_enforce_static_rules (
    const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats)
{

    std::string new_operation = update_job_demands ();

    if (new_operation != "") {
        change_in_system = true;
    }

    std::string operation = active_op;

    Logging::log_debug ("ControlApplication: Computing Static Rules ");

    if (change_in_system.load ()) {
        int current_jobs = job_location_tracker.size ();

        Logging::log_debug (
            "ControlApplication: Change in system: " + std::to_string (current_jobs));

        if (current_jobs > 0) {
            for (auto const& job_demand : job_demands) {
                if (job_demand.second != -1) {
                    job_rates[job_demand.first] = job_demand.second;
                }
            }

            std::unordered_map<std::string, bool> job_address_updated;
            for (auto const& app : job_location_tracker) {
                // validate if assigned rate surpasses the changing bandwidth threshold
                if (abs (job_rates[app.first] - job_previous_rates[app.first]) < IOPS_THRESHOLD) {
                    job_rates[app.first] = -1;
                } else {
                    send_enforcement_rule (app.first, app.second, operation, job_address_updated);
                }
            }

            /*Collect result from rules */
            collect_enforcement_rule_results (job_address_updated);
        }
    }
}

// compute_and_enforce_dynamic_vanilla_rules call. Computes and enforces DYNAMIC_VANILLA policies.
void CoreControlApplication::compute_and_enforce_dynamic_vanilla_rules (
    const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats)
{
    // Check if there is any change in the job's demands.
    std::string operation = update_job_demands ();

    // Check if there are active jobs to control.
    long current_jobs = job_location_tracker.size ();
    if (current_jobs > 0) {

        // Assign all bandwidth to leftover_iops
        long left_iops = maximum_limit;

        // First phase: For each job assign either fair share or demand.
        for (auto const& app : job_location_tracker) {

            // If job's demand is less than fair share, assign demand
            if (job_demands[app.first] <= (left_iops / current_jobs)) {
                if (job_demands[app.first] == -1) {
                    job_demands[app.first] = 1;
                }
                job_rates[app.first] = job_demands[app.first];
            } else {
                // If job's demand is greater than fair share, assign fair share
                job_rates[app.first] = (left_iops / current_jobs);
            }

            current_jobs--;

            // Consume assigned IOPS
            left_iops -= job_rates[app.first];
        }

        operation = active_op;
        current_jobs = job_location_tracker.size ();
        std::unordered_map<std::string, bool> job_address_updated;

        // Second phase: Distribute leftover bandwidth.
        for (auto const& app : job_location_tracker) {
            // Distribute equally any leftover bandwidth
            job_rates[app.first] += (left_iops / current_jobs);

            // Validate if assigned rate surpasses the changing bandwidth threshold
            if (!change_in_system.load ()
                && abs (job_rates[app.first] - job_previous_rates[app.first]) < IOPS_THRESHOLD) {
                job_rates[app.first] = -1;
            } else {
                send_enforcement_rule (app.first, app.second, operation, job_address_updated);
            }
        }

        /*Collect result from rules */
        collect_enforcement_rule_results (job_address_updated);
    }
}

// compute_and_enforce_dynamic_leftover_rules call. Computes and enforces DYNAMIC_LEFTOVER policies.
void CoreControlApplication::compute_and_enforce_dynamic_leftover_rules (
    const std::unordered_map<std::string, std::unique_ptr<StageResponse>>& d_stats,
    std::list<std::string>& sessions_sent)
{

    // Check if there is any change in the job's demands.
    std::string operation = update_job_demands ();

    // Check if there are active jobs to control.
    long current_jobs = job_location_tracker.size ();
    if (current_jobs > 0) {

        // Assign all bandwidth to leftover_iops
        unsigned long left_iops = maximum_limit;

        std::unordered_map<std::string, unsigned long> per_app_usage;
        unsigned long system_total_rate = 0;

        for (auto const& app : job_location_tracker) {
            long total_app_rate = 0;
            for (auto const& [local_address, envs] : app.second) {

                if (std::find (sessions_sent.begin (), sessions_sent.end (), local_address)
                    != sessions_sent.end ()) {
                    auto* response_ptr
                        = dynamic_cast<StageResponseStats*> (d_stats.at (local_address).get());

                    for (int env : envs) {
                        std::string stage_env = app.first + "+" + std::to_string (env);

                        auto stage_exists = (*response_ptr->m_stats_ptr.get ()).find (stage_env);

                        if (stage_exists == (*response_ptr->m_stats_ptr.get ()).end ()) {
                            long current_rate = 1;
                            total_app_rate += current_rate;
                        } else {
                            auto* entities_stat_ptr
                                = dynamic_cast<StageResponseStat*> (stage_exists->second.get ());

                            double current_rate = entities_stat_ptr->get_total_rate ();
                            if (current_rate == 0) {
                                current_rate = 1;
                            }

                            total_app_rate += current_rate;
                        }
                    }
                } else {
                    for (int env : envs) {
                        std::string stage_env = app.first + "+" + std::to_string (env);

                        double current_rate = maximum_limit * job_location_tracker.size ();
                        total_app_rate += current_rate;
                    }
                }
            }

            float fair_share = left_iops / current_jobs;

            // First phase
            if (total_app_rate <= job_demands[app.first] * 0.95) {
                float threshold_value = (float)(job_demands[app.first] - total_app_rate) * 0.25;

                if (total_app_rate + threshold_value < fair_share) {
                    job_rates[app.first] = total_app_rate + threshold_value;
                } else {
                    job_rates[app.first] = fair_share;
                }
            } else {
                if (job_demands[app.first] < fair_share) {
                    job_rates[app.first] = job_demands[app.first];
                } else {
                    job_rates[app.first] = fair_share;
                }
            }

            per_app_usage[app.first] = total_app_rate;
            system_total_rate += total_app_rate;
            current_jobs--;
            left_iops -= job_rates[app.first];
        }

        unsigned long left_iops_copy = left_iops;

        left_iops_copy = left_iops;

        // Second phase. Distribute leftover bandwidth according to current usage.
        if (left_iops_copy > 0) {
            for (auto const& app : per_app_usage) {
                float add_iops_perc = (float)app.second / system_total_rate;
                job_rates[app.first] += std::floor (add_iops_perc * left_iops_copy);
                left_iops -= std::floor (add_iops_perc * left_iops_copy);
            }
        }

        left_iops = 0;
        std::string operation = active_op;
        current_jobs = job_location_tracker.size ();
        std::unordered_map<std::string, bool> job_address_updated;
        std::unordered_map<std::string, bool> maintain_previous_job_rate;
        unsigned long maintained_rate = 0;
        unsigned long updated_rate = 0;

        // Distribute per each job's stage.
        for (auto const& app : job_location_tracker) {

            // validate if assigned rate surpasses the changing bandwidth threshold
            auto rates_difference = abs (job_rates[app.first] - job_previous_rates[app.first]);

            // validate if assigned rate surpasses the changing bandwidth threshold
            if (!change_in_system.load ()
                && (rates_difference < (job_rates[app.first] * 0.01)
                    || (system_total_rate < 0.95 * maximum_limit
                        && rates_difference < (job_rates[app.first] * 0.05)))) {
                maintained_rate += job_previous_rates[app.first];
                maintain_previous_job_rate[app.first] = true;

            } else {
                updated_rate += job_rates[app.first];
                maintain_previous_job_rate[app.first] = false;
            }
        }

        float varied_perc = 1.0;

        if (system_total_rate < 0.95 * maximum_limit
            && maintained_rate + updated_rate > maximum_limit) {
            long allowed_rate = maximum_limit - maintained_rate;
            varied_perc = (float)allowed_rate / updated_rate;
        }

        for (auto const& app : job_location_tracker) {
            if (maintain_previous_job_rate[app.first]) {
                /*DO Nothing*/

            } else {
                unsigned long cur_job_rate = job_rates[app.first];
                job_rates[app.first] = std::floor (cur_job_rate * varied_perc);

                send_enforcement_rule (app.first, app.second, operation, job_address_updated);
            }
        }

        /*Collect result rules */
        collect_enforcement_rule_results (job_address_updated);
    }
}

////////////////////////////////////////////
////// Enqueue and Dequeue new rules ///////
////////////////////////////////////////////

// enqueue_rule_in_queue call. Submit new rule to be imposed (used by the system administrator).
void CoreControlApplication::enqueue_rule_in_queue (const std::string& rule)
{
    pending_rules_queue_lock_.lock ();
    pending_rules_queue_.emplace (rule);
    pending_rules_queue_lock_.unlock ();
}

// dequeue_rule_from_queue call. Dequeues a new rule submitted by the system administrator.
std::string CoreControlApplication::dequeue_rule_from_queue ()
{
    std::string response_t;

    if (!pending_rules_queue_.empty ()) {
        pending_rules_queue_lock_.lock ();
        response_t = pending_rules_queue_.front ();
        pending_rules_queue_.pop ();
        pending_rules_queue_lock_.unlock ();
    }

    return response_t;
}

////////////////////////////////////////////
//////////// Auxiliary Functions ///////////
////////////////////////////////////////////

// Initialize call. Fills housekeeping rules and operations supported by the control application.
void CoreControlApplication::initialize ()
{
    for (const auto& specific_rule : *housekeeping_rules_ptr_) {
        std::vector<std::string> tokens {};

        parse_rule_with_break (specific_rule, &tokens);

        if (tokens[2] == "create_channel") {
            if (tokens[6] == "no_op"){
                active_ops.insert (tokens[7]);
            }
            else {
                active_ops.insert (tokens[6]);
            }
        }
    }

    active_op = *(active_ops.begin ());

    Logging::log_debug ("Current supported operations: ");
    for (const auto& op : active_ops) {
        Logging::log_debug (op + " ");
    }
}

// handle_local_controller_sessions call. Processes pending local controller session.
void CoreControlApplication::handle_local_controller_sessions ()
{
    if (!local_queue.empty ()) {
        pending_register_session_lock_.lock ();
        std::string local_controller_address = local_queue.front ();
        local_queue.pop ();
        pending_register_session_lock_.unlock ();

        m_pending_local_controller_sessions.fetch_sub (1);

        this->local_sessions_.emplace (local_controller_address,
            std::make_unique<LocalControllerSession> (local_controller_address));

        this->local_to_stages.emplace (local_controller_address, std::vector<std::string> {});

        std::thread session_thread_t = std::thread (&LocalControllerSession::StartSession,
            local_sessions_.at (local_controller_address).get (),
            local_controller_address);
        session_thread_t.detach ();

        PStatus status = this->local_handshake (local_controller_address);

        m_active_local_controller_sessions.fetch_add (1);
    }
}

// stage_handshake call. (...)
PStatus CoreControlApplication::local_handshake (const std::string& local_controller_address)
{
    Logging::log_debug (
        "CoreControlApplication: Local Controller Handshake (" + local_controller_address + ")");
    PStatus status = PStatus::Error ();

    if (local_sessions_[local_controller_address] != nullptr) {
        // invoke CallStageHandshake routine to acknowledge the stage's identifier
        status = this->call_local_handshake (local_controller_address);

    } else {
        Logging::log_info (
            "Local Controller Handshake: session (" + local_controller_address + ") is null.");
    }

    return status;
}

// call_stage_handshake call. Submits LOCAL_HANDSHAKE rule to a local controller.
PStatus CoreControlApplication::call_local_handshake (const std::string& local_controller_address)
{
    PStatus status = PStatus::Error ();
    // create LOCAL_HANDSHAKE request
    std::string rule = std::to_string (LOCAL_HANDSHAKE) + "|";
    // put request on LocalPlaneSession::submission_queue

    for (const auto& specific_rule : *housekeeping_rules_ptr_) {
        rule += ":" + specific_rule;
    }

    Logging::log_info ("LocalHandshake <" + local_controller_address + " " + rule + ">");

    local_sessions_[local_controller_address]->SubmitRule (rule);

    // wait for request to be on LocalPlaneSession::completion_queue
    std::unique_ptr<StageResponse> resp_t = local_sessions_[local_controller_address]->GetResult ();
    // convert StageResponse to Handshake object
    auto* ack_ptr_t = dynamic_cast<StageResponseACK*> (resp_t.get ());

    if (ack_ptr_t != nullptr) {
        if (ack_ptr_t->ACKValue () == static_cast<int> (AckCode::ok)) {
            status = PStatus::OK ();
        }
    }

    // Logging message
    Logging::log_info ("LocalHandshake </>");

    return status;
}

// handle_data_plane_sessions call. Processes pending data plane sessions.
void CoreControlApplication::handle_data_plane_sessions ()
{
    if (!local_to_data_queue_.empty ()) {
        pending_register_stage_lock_.lock ();
        auto stage_info = std::move (local_to_data_queue_.front ());
        local_to_data_queue_.pop ();
        pending_register_stage_lock_.unlock ();

        this->m_pending_data_plane_sessions.fetch_sub (1);

        change_in_system = true;

        auto existing_locations = job_location_tracker.find (stage_info->m_stage_name);

        if (existing_locations == job_location_tracker.end ()) {
            /* Does not exist */
            std::unordered_map<std::string, std::vector<int>> locations;

            std::vector<int> envs;
            envs.push_back (std::stoi (stage_info->m_stage_env));

            locations.emplace (stage_info->m_local_address, envs);
            job_location_tracker[stage_info->m_stage_name] = locations;

            job_demands.emplace (stage_info->m_stage_name, -1);
            job_previous_rates.emplace (stage_info->m_stage_name, 0);
            job_rates.emplace (stage_info->m_stage_name, -1);
        } else {
            /* Already exists */
            auto existing_envs = existing_locations->second.find (stage_info->m_local_address);

            if (existing_envs == existing_locations->second.end ()) {
                std::vector<int> envs;
                envs.push_back (std::stoi (stage_info->m_stage_env));
                existing_locations->second[stage_info->m_local_address] = envs;
            } else {
                existing_envs->second.push_back (std::stoi (stage_info->m_stage_env));
            }
        }

        std::string stage_name_env = stage_info->m_stage_name + "+" + stage_info->m_stage_env;

        auto existing_local_to_stages = local_to_stages.find (stage_info->m_local_address);
        if (existing_local_to_stages == local_to_stages.end ()) {
            std::vector<std::string> stages = {};
            stages.push_back (stage_name_env);
            local_to_stages[stage_info->m_local_address] = stages;
        } else {
            existing_local_to_stages->second.push_back (stage_name_env);
        }

        PStatus status = this->mark_data_plane_stage_ready (stage_info->m_local_address,
            stage_info->m_stage_name,
            stage_info->m_stage_env);

        //
        stage_info_detailed[stage_name_env] = std::move (stage_info);

        if (status.isOk ()) {
            this->m_active_data_plane_sessions.fetch_add (1);

            Logging::log_debug (
                "DataPlaneSessionHandshake with Data Plane successfully established.");
        } else {
            Logging::log_error ("DataPlaneSessionHandshake with Data Plane not established.");

            std::this_thread::sleep_for (milliseconds (100));
        }
    }
}

// mark_data_plane_stage_ready call. Submits STAGE_READY rule to a local controller to inform
// that the data plane stage identified by stage_name and stage_env has been register
// by the core controller.
PStatus CoreControlApplication::mark_data_plane_stage_ready (
    const std::string& local_controller_address,
    const std::string& stage_name,
    const std::string& stage_env)
{
    Logging::log_debug (
        "CoreControlApplication: mark stage ready (" + local_controller_address + ")");
    PStatus status = PStatus::Error ();

    std::string rule = std::to_string (STAGE_READY) + "|" + stage_name + "+" + stage_env + "|";
    // put request on DataPlaneSession::submission_queue
    local_sessions_.at (local_controller_address)->SubmitRule (rule);

    // get rules from CompletionQueue and cast them to a StageResponseACK object
    std::unique_ptr<StageResponse> resp_t
        = local_sessions_.at (local_controller_address)->GetResult ();
    auto* ack_ptr_t = dynamic_cast<StageResponseACK*> (resp_t.get ());

    if (ack_ptr_t != nullptr) {
        if (ack_ptr_t->ACKValue () == static_cast<int> (AckCode::ok)) {
            status = PStatus::OK ();
        }
    }

    return status;
}

// update_job_demands call. Process new rules and updates job's demands.
std::string CoreControlApplication::update_job_demands ()
{
    std::string new_rule = dequeue_rule_from_queue ();

    std::string operation;
    if (!new_rule.empty ()) {

        std::vector<std::string> tokens {};
        parse_rule_with_break (new_rule, &tokens);

        if (tokens[1] == "demand" || tokens[1] == "job") {
            std::string job_name = tokens[2];
            Logging::log_debug (
                "ControlApplication: Received rule for dynamic control with demands.");

            operation = tokens[3];
            auto it = job_demands.find (job_name);
            if (it != job_demands.end ()) {
                job_demands[job_name] = std::stol (tokens[4]);
            } else {
                job_demands.emplace (job_name, std::stol (tokens[4]));
            }
        }
    }

    active_op = operation;

    return operation;
}

// send_enforcement_rule call. Sends new enforcement rules to local controllers.
void CoreControlApplication::send_enforcement_rule (std::string app_name,
    std::unordered_map<std::string, std::vector<int>> local_to_envs,
    std::string operation,
    std::unordered_map<std::string, bool>& job_address_updated)
{
    job_previous_rates[app_name] = job_rates[app_name];

    int total_stages = 0;
    for (auto const& [local_address, envs] : local_to_envs) {
        total_stages += envs.size ();
    }

    long limit_per_stage = std::floor (job_rates[app_name] / total_stages);

    for (auto const& [local_address, envs] : local_to_envs) {

        std::string enforcement_rule = std::to_string (CREATE_ENF_RULE);
        enforcement_rule += "|.0|" + app_name + "|" + operation + "|";

        for (int env : envs) {
            std::string stage_env = app_name + "+" + std::to_string (env);

            enforcement_rule += "*" + std::to_string (env) + ":" + std::to_string (limit_per_stage);
        }

        enforcement_rule += "*.";

        job_address_updated[app_name + "+" + local_address] = true;

        Logging::log_debug ("Enforcing rule " + enforcement_rule + " to " + local_address);
        this->local_sessions_[local_address]->SubmitRule (enforcement_rule);
    }
}

// collect_enforcement_rule_results call . Collects enforcement rules results.
void CoreControlApplication::collect_enforcement_rule_results (
    std::unordered_map<std::string, bool>& job_address_updated)
{
    for (auto const& app : job_location_tracker) {
        for (auto const& [local_address, envs] : app.second) {
            if (job_address_updated[app.first + "+" + local_address]) {
                // get responses based on submitted rules
                std::unique_ptr<StageResponse> ack_ptr
                    = this->local_sessions_[local_address]->GetResult ();

                // debug message
                if (Logging::is_debug_enabled ()) {
                    // verify if pointer is valid
                    if (ack_ptr != nullptr) {
                        // convert StageResponse unique-ptr to StageResponseStats
                        auto* response_ptr = dynamic_cast<StageResponseACK*> (ack_ptr.get ());

                        Logging::log_debug (
                            "ACK response :: " + std::to_string (response_ptr->ResponseType ())
                            + " -- " + response_ptr->toString ());
                    }
                }
            }
        }
    }
}

// collect_statistics_result call. Collects a local controller statistics.
void CoreControlApplication::collect_statistics_result (
    const std::unique_ptr<LocalControllerSession>& session,
    std::string local_address,
    std::list<std::string>& sessions_to_delete,
    std::unordered_map<std::string, std::unique_ptr<StageResponse>>& collected_stats)
{
    std::unique_ptr<StageResponse> stats_ptr = session->GetResult ();

    auto* response_ptr = dynamic_cast<StageResponseStats*> (stats_ptr.get ());

    // verify if pointer is valid
    if (stats_ptr != nullptr && !response_ptr->m_stats_ptr.get ()->empty ()) {

        for (auto const& stats_value : (*response_ptr->m_stats_ptr.get ())) {
            auto* global_stat_ptr = dynamic_cast<StageResponseStat*> (stats_value.second.get ());

            double current_rate = global_stat_ptr->get_total_rate ();

            if (current_rate == -1) {
                remove_stage (stats_value.first);

                auto& stages = local_to_stages.at (local_address);
                stages.erase (std::remove (stages.begin (), stages.end (), stats_value.first),
                    stages.end ());

                if (stages.empty ()) {
                    local_to_stages.erase (local_address);
                    sessions_to_delete.push_back (local_address);
                }
            }
        }

        collected_stats.emplace (local_address, std::move (stats_ptr));
    } else {

        auto& stages = local_to_stages.at (local_address);

        for (const auto& stage : stages) {
            remove_stage (stage);
        }

        local_to_stages.erase (local_address);
        sessions_to_delete.push_back (local_address);
    }
}

// remove_stage call: Removes a data plane stage that is no longer operational.
void CoreControlApplication::remove_stage (const std::string& stage_name_env)
{
    auto stage_info = stage_info_detailed.at (stage_name_env).get ();

    std::unordered_map<std::string, std::vector<int>>& local_address_to_envs
        = job_location_tracker.at (stage_info->m_stage_name);
    std::vector<int>& envs = local_address_to_envs.at (stage_info->m_local_address);

    envs.erase (std::remove (envs.begin (), envs.end (), std::stoi (stage_info->m_stage_env)),
        envs.end ());

    if (envs.empty ()) {
        Logging::log_debug ("ControlApplication: Envs is empty");
        local_address_to_envs.erase (stage_info->m_local_address);

        if (local_address_to_envs.empty ()) {
            job_location_tracker.erase (stage_info->m_stage_name);
            job_rates.erase (stage_info->m_stage_name);
            job_previous_rates.erase (stage_info->m_stage_name);
            job_demands.erase (stage_info->m_stage_name);

            Logging::log_debug ("ControlApplication: No local sessions with app");
        }
    }

    stage_info_detailed.erase (stage_name_env);

    change_in_system = true;

    Logging::log_debug ("ControlApplication: Removing a data plane session.");
}

// parse_rule_with_break call. Parses a rule into tokens using '|' as delimiter.
void CoreControlApplication::parse_rule_with_break (const std::string& rule,
    std::vector<std::string>* tokens)
{
    size_t start;
    size_t end = 0;

    while ((start = rule.find_first_not_of ('|', end)) != std::string::npos) {
        end = rule.find ('|', start);
        tokens->push_back (rule.substr (start, end - start));
    }
}

} // namespace cheferd
