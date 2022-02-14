/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/controller/core_control_application.hpp>
#include <shepherd/utils/rules_file_parser.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
}

namespace shepherd {

// CoreControlApplication default constructor.
CoreControlApplication::CoreControlApplication (
    ControlType control_type) :
    ControlApplication {},
    m_control_type {control_type},
    local_sessions_ {},
    job_location_tracker{},
    job_rates{},
    job_previous_rates{},
    job_demands{},
    local_to_data_queue_ {},
    maximum_iops{0}
{
    Logging::log_info ("CoreControlApplication initialized.");
    initialize ();
}

// CoreControlApplication parameterized constructor.
CoreControlApplication::CoreControlApplication (
    ControlType control_type,
    std::vector<std::string>* rules_ptr,
    const uint64_t& cycle_sleep_time) :
    ControlApplication { rules_ptr, cycle_sleep_time },
    m_control_type {control_type},
    local_sessions_ {},
    job_rates{},
    job_previous_rates{},
    job_demands{},
    local_to_data_queue_ {},
    maximum_iops{0}
{
    Logging::log_info ("CoreControlApplication parameterized constructor.");
    initialize ();
}

//    CoreControlApplication default destructor.
CoreControlApplication::~CoreControlApplication ()
{
    std::cout << "CoreControlApplication: exiting ...\n";
}


//    Initialize call. (...)
void CoreControlApplication::initialize () {

    maximum_iops = 1073741824;

    /*for (int i = 0; i < MAX_TENSORFLOW_INSTANCES; i++) {
     //   m_rate[i] = 0;
     //   m_previous_rate[i] = 0;
     //   m_token_bucket_calibration[i] = 0;
    }*/
}


// CalibrateStageRate call. (...)
double CoreControlApplication::calibrate_stage_rate_factor (const int& index,
    const double& stage_statistic,
    const double& pid_statistic)
{
    if (pid_statistic > 0 && (stage_statistic - pid_statistic) > TOKEN_BUCKET_THRESHOLD) {
        m_token_bucket_calibration[index]++;
        return 1.0 + ((double)m_token_bucket_calibration[index] / 10);
    } else {
        m_token_bucket_calibration[index] = 0;
        return 1.0;
    }
}

//    RegisterDataPlaneSession call. (...)
void CoreControlApplication::register_stage_session (const std::string& local_controller_address,
    const std::string& stage_name,
    const std::string& stage_env,
    const std::string& stage_user)
{
    Logging::log_debug ("RegisterDataPlaneSession --" + local_controller_address + " : " + stage_name + "+" + stage_env);

    //session_array_[index] = make_unique<DataPlaneSession> ();
    int local_index = this->local_address_to_index_.at(local_controller_address);
    this->local_session_array_.at(local_index).emplace_back(stage_name);
    this->m_pending_data_plane_sessions.fetch_add (1);


    auto existing_locations = job_location_tracker.find(stage_name);

    if ( existing_locations == job_location_tracker.end() ) {
        /* Does not exist */
        std::unordered_map<int, std::vector<int>> locations;

        std::vector<int> envs;
        envs.push_back(std::stoi(stage_env));

        locations.emplace(local_index, envs);
        job_location_tracker [stage_name] = locations;

        job_demands.emplace(stage_name, 0);
        job_previous_rates.emplace(stage_name, 0);
        job_rates.emplace(stage_name, 0);
    }
    else {
        /* Already exists */
        auto existing_envs = existing_locations->second.find(local_index);

        if ( existing_envs == existing_locations->second.end() ) {
            std::vector<int> envs;
            envs.push_back(std::stoi(stage_env));
            existing_locations->second[local_index] = envs;
        }
        else {
            existing_envs->second.push_back(std::stoi(stage_env));
        }
    }

    std::pair<int,std::string> local_to_index_info = std::make_pair( local_index, stage_name );
    local_to_data_queue_.push(local_to_index_info);
}


//    RegisterLocalControllerSession call. (...)
LocalControllerSession* CoreControlApplication::register_local_controller_session (const std::string& local_controller_address, int local_index)
{
    Logging::log_debug ("RegisterLocalControllerSession -- " + local_controller_address);

    this->local_sessions_.push_back(std::make_unique<LocalControllerSession> (local_controller_address));

    this->local_address_to_index_.emplace(local_controller_address,  local_index);

    this->local_session_array_.emplace(local_index, std::vector<std::string> {});

    this->m_pending_local_controller_sessions.fetch_add (1);

    return local_sessions_[local_index].get();
}

// stage_handshake call. (...)
PStatus CoreControlApplication::local_handshake (int local_index)
{
    Logging::log_debug (
        "CoreControlApplication: Local Controller Handshake (" + std::to_string (local_index) + ")");
    PStatus status = PStatus::Error ();

    if (local_sessions_[local_index] != nullptr) {
        // invoke CallStageHandshake routine to acknowledge the stage's identifier
        status = this->call_local_handshake (local_index);

    } else {
        Logging::log_info (
            "Local Controller Handshake: session (" + std::to_string (local_index) + ") is null.");
    }

    return status;
}



// operator call. (...)
void CoreControlApplication::operator () ()
{
    this->execute_feedback_loop ();
}


// execute_feedback_loop call. (...)
void CoreControlApplication::execute_feedback_loop ()
{
    Logging::log_debug ("CoreControlApplication::ExecuteFeedbackLoop");
    PStatus status = PStatus::Error ();
    int index = 0;

    // wait for a data plane stage to connect
    while (this->m_pending_local_controller_sessions.load () == 0) {
        std::this_thread::sleep_for (milliseconds (100));
    }

    // while existing data plane stage connections (active or pending), execute the feedback loop
    while ((this->m_pending_local_controller_sessions.load () + this->m_active_local_controller_sessions.load () + this->m_pending_data_plane_sessions.load ()) > 0 ) {
        // if exists pending sessions, perform the Session Handshake
        while (this->m_pending_local_controller_sessions.load () > 0) {
            // execute session handshake
            status = this->local_handshake(index);

            if (status.isOk ()) {
                index++;
                this->m_pending_local_controller_sessions.fetch_sub (1);
                this->m_active_local_controller_sessions.fetch_add (1);

                Logging::log_debug ("LocalControllerSessionHandshake with Local Controller-"
                    + std::to_string (index) + " successfully established.");
            } else {
                Logging::log_error ("LocalControllerSessionHandshake with Local Controller-"
                    + std::to_string (index + 1) + " not established.");

                std::this_thread::sleep_for (milliseconds (100));
            }
        }

        while (this->m_pending_data_plane_sessions.load () > 0) {
            std::pair<int,std::string> stage_info = local_to_data_queue_.front();
            local_to_data_queue_.pop();

            status = PStatus::OK ();

            if (status.isOk ()) {
                index++;
                this->m_pending_data_plane_sessions.fetch_sub (1);
                this->m_active_data_plane_sessions.fetch_add (1);

                Logging::log_debug ("DataPlaneSessionHandshake with TensorFlow-"
                                    + std::to_string (index) + " successfully established.");
            } else {
                Logging::log_error ("DataPlaneSessionHandshake with TensorFlow-"
                                    + std::to_string (index + 1) + " not established.");

                std::this_thread::sleep_for (milliseconds (100));
            }

        }

        // 2nd phase: compute statistics and enforce rules


        switch (m_control_type){
            case ControlType::STATIC: {
                const std::unordered_map<int, std::unique_ptr<StageResponse>>& s_stats
                    = this->collect_statistics_global (this->m_active_local_controller_sessions.load (), 0);
                this->compute_and_enforce_static_rules(s_stats, this->m_active_local_controller_sessions.load (), 0);
                break;
            }
            case ControlType::DYNAMIC: {
                const std::unordered_map<int, std::unique_ptr<StageResponse>>& d_stats
                    = this->collect_statistics_global (this->m_active_local_controller_sessions.load (), 0);

                this->compute_and_enforce_dynamic_rules(d_stats, this->m_active_local_controller_sessions.load (), 0);
                break;
            }
            case ControlType::MDS: {
                const std::unordered_map<int, std::unique_ptr<StageResponse>>& entity_stats
                    = this->collect_statistics_entity (this->m_active_local_controller_sessions.load (), 0);

                this->compute_and_enforce_mds_rules (entity_stats, this->m_active_local_controller_sessions.load (), 0);
                break;
            }
            default:
                break;
        }
        // 2nd phase: compute statistics and enforce rules
        // Compute (tf_stats, active_data_plane_sessions.load(), index - active_data_plane_sessions)
        //this->compute (tf_stats, this->m_active_local_controller_sessions.load (), 0);

        // 3rd phase: sleep for the next feedback loop cycle
        this->sleep ();
    }

    // log message and end control loop
    Logging::log_info ("Exiting. No active connections.");
    _exit (EXIT_SUCCESS);
}

// call_stage_handshake call. (...)
PStatus CoreControlApplication::call_local_handshake (const int& local_index)
{
    PStatus status = PStatus::Error ();
    // create LOCAL_HANDSHAKE request
    std::string rule = std::to_string (LOCAL_HANDSHAKE) + "|";
    // put request on LocalPlaneSession::submission_queue

    for (const auto &specific_rule : *housekeeping_rules_ptr_){
        rule += ":" + specific_rule;
    }

    Logging::log_info ("LocalHandshake <" + rule +">");


    local_sessions_[local_index]->SubmitRule (rule);

    // wait for request to be on LocalPlaneSession::completion_queue
    std::unique_ptr<StageResponse> resp_t = local_sessions_[local_index]->GetResult ();
    // convert StageResponse to Handshake object
    auto* ack_ptr_t = dynamic_cast<StageResponseACK*> (resp_t.get ());

    if (ack_ptr_t != nullptr) {
        if (ack_ptr_t->ACKValue () == static_cast<int> (AckCode::ok)) {
            status = PStatus::OK ();
        }
    }

    // register instance PID
    //if (handshake_ptr != nullptr) {
      //  m_instance_pid[index] = handshake_ptr->get_stage_pid ();
   // }

    // Logging message
    Logging::log_info ("LocalHandshake <>");

    return status;
}


//    CollectStatistics call. (...)
std::unique_ptr<StageResponse> CoreControlApplication::collect_statistics ()
{
    return nullptr;
}


// collect_statistics call. (...)
std::unordered_map<int, std::unique_ptr<StageResponse>>
CoreControlApplication::collect_statistics_global (const int& active_sessions, const int& start_index)
{
    Logging::log_debug ("ControlApplication:collect_statistics_global");

    std::unordered_map<int, std::unique_ptr<StageResponse>> collected_stats {};

    // create COLLECT_STATS_TF request
    std::string rule = std::to_string (COLLECT_GLOBAL_STATS) + "|";

    // submit requests to each DataPlaneSession's submission_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // put request on DataPlaneSession::submission_queue
        local_sessions_[i]->SubmitRule (rule);
    }

    // collect requests from each DataPlaneSession's completion_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // wait for request to be on DataPlaneSession::completion_queue
        std::unique_ptr<StageResponse> stats_ptr = local_sessions_[i]->GetResult ();

        // verify if pointer is valid
        if (stats_ptr != nullptr) {
            collected_stats.emplace(i,std::move(stats_ptr));
        }
        else {
            Logging::log_error ("collect_statistics_entity: Connection error; disconnecting from instance-" +
                std::to_string (i));
            this->m_active_local_controller_sessions.fetch_sub (1);
        }
    }

    return collected_stats;
}

// collect_statistics call. (...)
std::unordered_map<int, std::unique_ptr<StageResponse>>
CoreControlApplication::collect_statistics_entity (const int& active_sessions, const int& start_index)
{
    Logging::log_debug ("ControlApplication:collect_statistics_entity");

    std::unordered_map<int, std::unique_ptr<StageResponse>> collected_stats {};

    // create COLLECT_STATS_TF request
    std::string rule = std::to_string (COLLECT_ENTITY_STATS) + "|";

    // submit requests to each DataPlaneSession's submission_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // put request on DataPlaneSession::submission_queue
        local_sessions_[i]->SubmitRule (rule);
    }


    // collect requests from each DataPlaneSession's completion_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // wait for request to be on DataPlaneSession::completion_queue
        std::unique_ptr<StageResponse> stats_ptr = local_sessions_[i]->GetResult ();


        // verify if pointer is valid
        if (stats_ptr != nullptr) {
            collected_stats.emplace(i,std::move(stats_ptr));
        }
        else {
            Logging::log_error ("collect_statistics_entity: Connection error; disconnecting from instance-" +
                std::to_string (i));
            this->m_active_local_controller_sessions.fetch_sub (1);
        }

    }

    return collected_stats;
}

//    Compute call. (...)
void CoreControlApplication::compute (const std::unique_ptr<StageResponse>& statistics_ptr)
{
    Logging::log_debug ("CoreControlApplication::Compute");
}


// sleep call. (...)
void CoreControlApplication::sleep ()
{
    std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time));
}


void CoreControlApplication::compute_and_enforce_static_rules (
    const std::unordered_map<int, std::unique_ptr<StageResponse>>& s_stats,
    const int& active_sessions,
    const int& start_index)
{

    std::string static_rule = DequeueRuleFromQueue();

    if (!static_rule.empty()){

        std::vector<std::string> tokens {};
        size_t start;
        size_t end = 0;

        while ((start = static_rule.find_first_not_of ('|', end)) != std::string::npos) {
            end = static_rule.find ('|', start);
            tokens.push_back (static_rule.substr (start, end - start));
        }

        if (tokens[1].compare("job") == 0) {
            std::string job_name = tokens[2];
            Logging::log_debug ("ControlApplication: Received static rule for job.");

            auto existing_locations = job_location_tracker.find (job_name);

            if (existing_locations == job_location_tracker.end ()) {
                /*App does not exist in system*/
                /*TO-DO: Colocar valores guardados */
            }
            else {

                int total_stages = 0;

                for (auto const& [local_index, envs] : existing_locations->second) {
                    total_stages += envs.size ();
                }

                if (total_stages){
                    /*Compute iops for each stage*/
                    long limit = std::stol (tokens[4]);

                    long limit_per_stage = std::floor (limit / total_stages);

                    Logging::log_debug ("ControlApplication: Computing static rules. Limit per stage: " + std::to_string (limit_per_stage) + ".");

                    /*Enforce static rules */
                    for (auto const& [local_index, envs] : existing_locations->second) {

                        std::string enforcement_rule = std::to_string (CREATE_ENF_RULE) + "|"
                            + tokens[0] + "|" // rule-id
                            + job_name + "|" // ex: tensor; kvs
                            + tokens[3] + "|"; // operation

                        //auto* response_ptr = dynamic_cast<StageResponseStats*> (s_stats.at(local_index).get ());


                        for (int env : envs) {
                            std::string stage_env = job_name + "+" + std::to_string (env);

                            //auto* entities_stat_ptr = dynamic_cast<StageResponseStatsGlobal*> ((*response_ptr->m_stats_ptr.get()).get().at(stage_env));

                            //double current_rate = entities_stat_ptr->get_read_rate();

                            //Logging::log_debug ("Stats[" + stage_env + ", "+ std::to_string(current_rate) + "]");

                            enforcement_rule
                                += "*" + std::to_string (env) + ":" + std::to_string (limit_per_stage);
                        }

                        enforcement_rule += "*";

                        this->local_sessions_[local_index]->SubmitRule (enforcement_rule);
                    }

                    for (auto const& [local_index, envs] : existing_locations->second) {
                        // get responses based on submitted rules
                        std::unique_ptr<StageResponse> ack_ptr = this->local_sessions_[local_index]->GetResult ();

                        // debug message
                        if (Logging::is_debug_enabled ()) {
                            // verify if pointer is valid
                            if (ack_ptr != nullptr) {
                                // convert StageResponse unique-ptr to StageResponseStatsKVS
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
    }
}

void CoreControlApplication::compute_and_enforce_dynamic_rules (
    const std::unordered_map<int, std::unique_ptr<StageResponse>>& d_stats,
    const int& active_sessions,
    const int& start_index)
{

    std::string dynamic_rule = DequeueRuleFromQueue();

    if (!dynamic_rule.empty()) {

        std::vector<std::string> tokens {};
        size_t start;
        size_t end = 0;

        while ((start = dynamic_rule.find_first_not_of ('|', end)) != std::string::npos) {
            end = dynamic_rule.find ('|', start);
            tokens.push_back (dynamic_rule.substr (start, end - start));
        }

        if (tokens[1].compare ("demand") == 0) {
            std::string job_name = tokens[2];
            Logging::log_debug (
                "ControlApplication: Received rule for dynamic control with demands.");

            job_demands.emplace(job_name, std::stol(tokens[4]));
        }
    }

    int current_jobs = job_location_tracker.size();

    // Logging::log_debug ("CoreControlApplication::Compute");
    bool calibrate_bucket = option_calibrate_token_buckets;

    // assign max bandwidth of the control application
    long left_iops = maximum_iops;

    for(auto const& app :job_location_tracker ) {
        // if instance[index]'s demand is less than fair share
        if (job_demands[app.first]
            <= (left_iops / current_jobs)) {
            job_rates[app.first] = job_demands[app.first];
        } else {
            // if instance[index]'s demand is greater than fair share
            job_rates[app.first] = (left_iops / current_jobs);
        }

        current_jobs--;

        // consume iops
        left_iops -= job_rates[app.first];

        // debug message
        // Logging::log_debug ("Rate[" + std::to_string (index) + "]: " + std::to_string (this->m_rate[index] / 1024 / 1024));
        // std::cout << "Rate[" << index << "]: " << this->m_rate[index] / 1024 / 1024 << "; leftover bw: " << left_bandwidth_bps / 1024 / 1024 << "\n";
    }

    current_jobs = job_location_tracker.size();

    for(auto const& app :job_location_tracker ) {
        // update bandwidth
        job_rates[app.first] += (left_iops / current_jobs);

        // verify if Token-Bucket calibration is enabled
        if (calibrate_bucket) {

            // get calibrate factor for each job
            double calibrate = 0;
            /*this->calibrate_stage_rate_factor (index,
            //tf_stats[index].m_instance_read_bandwidth,
            tf_stats[index].m_pid_read_bandwidth);*/

            // calibrate rate
            job_rates[app.first] = ((double)job_rates[app.first] * calibrate);

            // debug message
            Logging::log_debug ("Calibrate[" + app.first + ", "
                + std::to_string (calibrate) + "]: " + std::to_string (job_rates[app.first]));
        }

        // validate if assigned rate surpasses the changing bandwidth threshold
        if (abs (job_rates[app.first] - job_previous_rates[app.first]) < IOPS_THRESHOLD) {
            job_rates[app.first] = -1;
            // std::cout << "RateFinal[" << index << "]: " << -1 << "\n";
        } else {
            job_previous_rates[app.first] = job_rates[app.first];
            // std::cout << "RateFinal[" << index << "]: " << this->m_rate[index] / 1024 / 1024 << "\n";
        }

        // debug message
        Logging::log_debug ("RateFinal[" + app.first
            + "]: " + std::to_string (job_rates[app.first]));
    }

}

void CoreControlApplication::compute_and_enforce_mds_rules (
    const std::unordered_map<int, std::unique_ptr<StageResponse>>& entity_stats,
    const int& active_sessions,
    const int& start_index)
{

    std::string mds_rule = DequeueRuleFromQueue();

    if (!mds_rule.empty()){

        std::vector<std::string> tokens {};
        size_t start;
        size_t end = 0;

        while ((start = mds_rule.find_first_not_of ('|', end)) != std::string::npos) {
            end = mds_rule.find ('|', start);
            tokens.push_back (mds_rule.substr (start, end - start));
        }

        if (tokens[1].compare("mds") == 0) {
            Logging::log_debug ("ControlApplication: Received rule for mds.");

            int total_stages = this->m_active_data_plane_sessions.load ();

            if (total_stages > 0){
                /*Compute iops for each stage*/
                long limit = std::stol (tokens[4]);

                long limit_per_stage = std::floor (limit / total_stages);

                Logging::log_debug ("ControlApplication: Computing mds rules. Limit mds " + tokens[2] + " per stage: " + std::to_string (limit_per_stage) + ".");


                /*Enforce mds rules */
                for(auto const& app :job_location_tracker ) {
                    for (auto const& [local_index, envs] : app.second) {

                        //auto* response_ptr = dynamic_cast<StageResponseStats*> (entity_stats.at(local_index).get ());


                        std::string enforcement_rule = std::to_string (CREATE_ENF_RULE) + "|"
                            + tokens[0] + "|" // rule-id
                            + app.first + "|" // app: ex: tensor, kvs
                            + tokens[2] + "|"; // op, ex: mds1

                        for (int env : envs) {
                            std::string stage_env = app.first + "+" + std::to_string (env);

                            //auto* entities_stat_ptr = dynamic_cast<StageResponseStatsEntity*> ((*response_ptr->m_stats_ptr.get()).at(stage_env).get());

                            //double current_rate = entities_stat_ptr->entity_rates.get()->at(tokens[2]);

                            //Logging::log_debug ("Stats[" + stage_env + ", "+ std::to_string(current_rate) + "]");

                            enforcement_rule
                                += "*" + std::to_string (env) + ":" + std::to_string (limit_per_stage);

                        }

                        enforcement_rule += "*";

                        this->local_sessions_[local_index]->SubmitRule (enforcement_rule);
                    }

                    for (auto const& [local_index, envs] : app.second) {
                        // get responses based on submitted rules
                        std::unique_ptr<StageResponse> ack_ptr = this->local_sessions_[local_index]->GetResult ();

                        // debug message
                        if (Logging::is_debug_enabled ()) {
                            // verify if pointer is valid
                            if (ack_ptr != nullptr) {
                                // convert StageResponse unique-ptr to StageResponseStatsKVS
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
    }
}


// Missing: probably some marshaling and unmarshaling needs to be done in this method
//  EnqueueResponseInCompletionQueue call. Enqueue response in the completion_queue.
void CoreControlApplication::EnqueueRuleInQueue (
    std::string rule)
{
    pending_rules_queue_lock_.lock();
    pending_rules_queue_.emplace (rule);
    pending_rules_queue_lock_.unlock();
}

// Missing: add wait_for and cv_status to exit the condition when we need to terminate the execution
//  DequeueResponseFromCompletionQueue call. Dequeue response from the completion_queue.
std::string CoreControlApplication::DequeueRuleFromQueue ()
{
    std::string response_t ="";

    if (!pending_rules_queue_.empty ()){
        pending_rules_queue_lock_.lock();
        response_t = pending_rules_queue_.front ();
        pending_rules_queue_.pop ();
        pending_rules_queue_lock_.unlock();
    }

    //std::string response_t = std::move (pending_rules_queue_.front ());

    return response_t;
}

} // namespace shepherd