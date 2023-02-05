/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/local_control_application.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
}

namespace cheferd {

// LocalControlApplication parameterized constructor.
LocalControlApplication::LocalControlApplication (const std::string& core_address,
    const std::string& local_address) :
    ControlApplication {},
    data_sessions_ {},
    preparing_data_sessions_ {},
    pending_data_sessions_ {},
    operation_to_channel_object {},
    core_stub_ (LocalToGlobal::NewStub (
        grpc::CreateChannel (core_address, grpc::InsecureChannelCredentials ()))),
    m_active_data_plane_sessions { 0 },
    m_pending_data_plane_sessions { 0 }
{
    Logging::log_info ("LocalControlApplication initialized.");
    initialize ();
    this->local_address = local_address;

    std::thread control_application_thread_t
        = std::thread (&LocalControlApplication::RunGlobalToLocalServer, this);
    control_application_thread_t.detach ();

    ConnectLocalToGlobal ();
}

// LocalControlApplication parameterized constructor.
LocalControlApplication::LocalControlApplication (std::vector<std::string>* rules_ptr,
    const std::string& core_address,
    const std::string& local_address,
    const uint64_t& cycle_sleep_time) :
    ControlApplication { rules_ptr, cycle_sleep_time },
    data_sessions_ {},
    preparing_data_sessions_ {},
    pending_data_sessions_ {},
    operation_to_channel_object {},
    core_stub_ (LocalToGlobal::NewStub (
        grpc::CreateChannel (core_address, grpc::InsecureChannelCredentials ()))),
    m_active_data_plane_sessions { 0 },
    m_pending_data_plane_sessions { 0 }
{
    Logging::log_info ("LocalControlApplication parameterized constructor.");

    initialize ();
    this->local_address = local_address;

    std::thread control_application_thread_t
        = std::thread (&LocalControlApplication::RunGlobalToLocalServer, this);
    control_application_thread_t.detach ();

    ConnectLocalToGlobal ();
}

// LocalControlApplication default destructor.
LocalControlApplication::~LocalControlApplication ()
{
    Logging::log_info ("LocalControlApplication: exiting ...\n");
}

////////////////////////////////////////////
///////// Register new sessions ////////////
////////////////////////////////////////////

//    RegisterDataPlaneSession call. (...)
void LocalControlApplication::register_stage_session (int socket_t)
{
    Logging::log_debug ("RegisterDataPlaneSession -- DataPlaneStage-" + std::to_string (socket_t));

    pending_data_plane_sessions_lock_.lock ();
    pending_data_sessions_.emplace (std::make_unique<HandshakeSession> (socket_t));

    pending_data_plane_sessions_lock_.unlock ();

    this->m_pending_data_plane_sessions.fetch_add (1);
}

////////////////////////////////////////////
/////////////// Feedback Loop //////////////
////////////////////////////////////////////

// operator call. (...)
void LocalControlApplication::operator() ()
{
    this->execute_feedback_loop ();
}

void LocalControlApplication::stop_feedback_loop ()
{
    server->Shutdown ();
    working_application_ = false;
    while (!pending_data_sessions_.empty ()) {
        pending_data_sessions_.front ()->RemoveSession ();
        pending_data_sessions_.pop ();
    }
    for (auto const& data_session : data_sessions_) {
        data_session.second->RemoveSession ();
    }
}

// execute_feedback_loop call.  Executes feedback loop.
void LocalControlApplication::execute_feedback_loop ()
{
    Logging::log_debug ("LocalControlApplication::ExecuteFeedbackLoop");
    PStatus status = PStatus::Error ();
    working_application_ = true;

    // wait for a data plane stage to connect
    while (working_application_.load () && this->m_pending_data_plane_sessions.load () == 0) {
        std::this_thread::sleep_for (milliseconds (100));
    }

    // while existing data plane stage connections (active or pending), execute the feedback loop
    while (working_application_.load ()
        && (this->m_pending_data_plane_sessions.load () > 0
            || this->m_active_data_plane_sessions.load () > 0)) {
        // if exists pending sessions, perform the Session Handshake
        while (this->m_pending_data_plane_sessions.load () > 0) {
            // execute session handshake
            handle_data_plane_sessions ();
            std::this_thread::sleep_for (milliseconds (100));
        }

        this->sleep ();
    }

    working_application_ = false;

    // log message and end control loop
    Logging::log_info ("Exiting. No active connections.");
    _exit (EXIT_SUCCESS);
}

////////////////////////////////////////////
//////////////// Sleep /////////////////////
////////////////////////////////////////////

// sleep call. Used to make control application main thread wait for the next loop.
void LocalControlApplication::sleep ()
{
    std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time));
}

//////////////////////////////////////////////
/// Local to core controller communication ///
//////////////////////////////////////////////

// RunGlobalToLocalServer call. Execute server to core controller communicate to local controller.
void LocalControlApplication::RunGlobalToLocalServer ()
{
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort (local_address, grpc::InsecureServerCredentials ());
    // Register "service" as the instance through which we'll communicate with clients.
    // In this case it corresponds to an *synchronous* service.
    builder.RegisterService (this);
    // Finally assemble the server.
    server = builder.BuildAndStart ();
    Logging::log_info ("LocalControlApplication: Server listening on " + local_address);

    // Wait for the server to shutdown.
    // Note that some other thread must be responsible for shutting down the server for this call to
    // ever return.
    server->Wait ();
}

Status LocalControlApplication::ConnectLocalToGlobal ()
{
    // Data we are sending to the server.
    ConnectRequest request;
    request.set_user_address (local_address);

    // Container for the data we expect from the server.
    ConnectReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = core_stub_->ConnectLocalToGlobal (&context, request, &reply);

    Logging::log_info (
        "LocalControlApplication: Connect local controller request sent to global controller");

    if (status.ok ()) {
        Logging::log_info ("LocalControlApplication: Connection successful");
    } else {
        Logging::log_info ("LocalControlApplication: Connection failed");
        // std::cout << status.error_code () << ": " << status.error_message () << std::endl;
    }
    return status;
}

// ConnectStageToGlobal call. Connect data plane stage  to core controller.
Status LocalControlApplication::ConnectStageToGlobal (const std::string& stage_name,
    const std::string& stage_env,
    const std::string& stage_user)
{
    // Data we are sending to the server.
    StageInfoConnect request;
    request.set_local_address (local_address);
    request.set_stage_name (stage_name);
    request.set_stage_env (stage_env);
    request.set_stage_user (stage_user);

    // Container for the data we expect from the server.
    ConnectReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = core_stub_->ConnectStageToGlobal (&context, request, &reply);

    Logging::log_info ("LocalControlApplication: Connect stage request sent to global controller");

    if (status.ok ()) {
        Logging::log_info ("LocalControlApplication: Connection successful");
    } else {
        Logging::log_info ("LocalControlApplication: Connection failed");
        // std::cout << status.error_code () << ": " << status.error_message () << std::endl;
    }

    return status;
}

//////////////////////////////////////////////
/// Core to local controller communication ///
//////////////////////////////////////////////

// LocalHandshake call. Local controller handshake from core controller.
Status LocalControlApplication::LocalHandshake (ServerContext* context,
    const controllers_grpc_interface::LocalSimplifiedHandshakeRaw* request,
    controllers_grpc_interface::ACK* reply)
{
    Logging::log_info ("LocalControlApplication: Received local handshake from core controller");

    for (auto& rule : request->rules ()) {
        Logging::log_info ("LocalControlApplication: " + rule + "\n");

        housekeeping_rules_ptr_->push_back (rule);

        std::vector<std::string> rule_tokens {};
        parse_rule (rule, &rule_tokens, '|');

        if (rule_tokens[2].compare ("create_channel") == 0) {
            auto channel_object = std::make_pair (std::stoi (rule_tokens[3]), 1);

            auto channels_objects = operation_to_channel_object.find (rule_tokens[6]);

            if (channels_objects == operation_to_channel_object.end ()) {

                std::vector<std::pair<int, int>> new_channels_objects;
                new_channels_objects.push_back (channel_object);
                operation_to_channel_object.emplace (rule_tokens[6], new_channels_objects);
            } else {
                channels_objects->second.push_back (channel_object);
            }
        }
    }

    reply->set_m_message (1);
    return Status::OK;
}

// StageHandshake call. Stage handshake from core controller.
Status LocalControlApplication::StageHandshake (ServerContext* context,
    const controllers_grpc_interface::ControlOperation* request,
    controllers_grpc_interface::StageSimplifiedHandshakeRaw* reply)
{
    Logging::log_info ("LocalControlApplication: Received stage handshake from core controller");
    return Status::OK;
}

Status LocalControlApplication::MarkStageReady (ServerContext* context,
    const controllers_grpc_interface::StageReadyRaw* request,
    controllers_grpc_interface::ACK* reply)
{
    Logging::log_info ("LocalControlApplication: Mark stage ready " + request->stage_name_env ()
        + " from core controller");

    auto stage = preparing_data_sessions_.extract (request->stage_name_env ());
    data_sessions_.insert (std::move (stage));

    preparing_data_sessions_.erase (request->stage_name_env ());

    reply->set_m_message (1);
    return Status::OK;
}

// CreateEnforcementRule call. Create enforcement from core controller.
Status LocalControlApplication::CreateEnforcementRule (ServerContext* context,
    const controllers_grpc_interface::EnforcementRules* request,
    controllers_grpc_interface::ACK* reply)
{
    Logging::log_info (
        "LocalControlApplication: Received create enforcement rule from core controller");

    Status status = Status::OK;

    for (auto& operation_rates : request->operation_rules ()) {

        for (auto& env_rate : operation_rates.second.env_rates ()) {

            auto existing_channels = operation_to_channel_object.find (operation_rates.first);

            if (existing_channels == operation_to_channel_object.end ()) {
                reply->set_m_message (0);
                status = Status::CANCELLED;
            } else {

                int total_channels = existing_channels->second.size ();
                long limit_per_channel = 0;
                if (total_channels > 0) {
                    limit_per_channel = std::floor (env_rate.second / total_channels);
                }

                for (auto& channel_objects : existing_channels->second) {

                    int channel_id = channel_objects.first;
                    int enforcement_object_id = channel_objects.second;
                    // TO-DO: Remove this
                    if (channel_id == 2000) {
                        std::string enforcement_rule = std::to_string (CREATE_ENF_RULE) + "|"
                            + std::to_string (operation_rates.second.m_rule_id ()) + "|" // rule-id
                            + std::to_string (channel_id) + "|"
                            + std::to_string (enforcement_object_id) + "|" + "drl" + "|" + "rate"
                            + "|" + std::to_string (limit_per_channel);

                        status = LocalPassthru (operation_rates.second.m_stage_name () + "+"
                                + std::to_string (env_rate.first),
                            enforcement_rule);

                        if (status.ok ()) {
                            reply->set_m_message (1);
                        } else {
                            // reply->set_m_message(0);
                            // return status;
                            reply->set_m_message (1);
                            return Status::OK;
                        }
                    }
                }
            }
        }
    }

    return status;
}

// CollectGlobalStatistics call. Collect Statistics request from core controller.
Status LocalControlApplication::CollectGlobalStatistics (ServerContext* context,
    const controllers_grpc_interface::ControlOperation* request,
    controllers_grpc_interface::StatsGlobalMap* reply)
{
    Logging::log_info ("LocalControlApplication: Received collect statistics "
                       "request from core controller");

    std::string rule = std::to_string (COLLECT_DETAILED_STATS) + "|"
        + std::to_string (COLLECT_GLOBAL_STATS) + "|";

    // submit requests to each DataPlaneSession's submission_queue
    for (auto const& data_session : data_sessions_) {
        // put request on DataPlaneSession::submission_queue
        data_session.second->SubmitRule (rule);
    }

    auto& stats_map = *reply->mutable_gl_stats ();

    std::list<std::string> sessions_to_delete;

    // collect requests from each DataPlaneSession's completion_queue
    for (auto const& data_session : data_sessions_) {

        // wait for request to be on DataPlaneSession::completion_queue
        std::unique_ptr<StageResponse> stats_ptr = data_session.second->GetResult ();

        // verify if pointer is valid
        if (stats_ptr != nullptr) {
            // convert StageResponse unique-ptr to StageResponseStatsKVS
            auto* response_ptr = dynamic_cast<StageResponseStat*> (stats_ptr.get ());

            if (response_ptr->get_total_rate () == -1) {
                Logging::log_info ("LocalControlApplication: CollectGlobalStatistics ->"
                                   "Connection error; disconnecting from instance-"
                    + data_session.first);
                this->m_active_data_plane_sessions.fetch_sub (1);
                sessions_to_delete.push_back (data_session.first);
            }

            std::string name = data_session.first;

            controllers_grpc_interface::StatsGlobal stats_global;

            stats_global.set_m_metadata_total_rate (response_ptr->get_total_rate ());

            stats_map[name] = stats_global;

        } else {
            Logging::log_info ("LocalControlApplication: CollectGlobalStatistics ->"
                               "Connection error; disconnecting from instance-"
                + data_session.first);
            this->m_active_data_plane_sessions.fetch_sub (1);
        }
    }

    for (auto const& data_session : sessions_to_delete) {
        Logging::log_info (
            "LocalControlApplication: Deleting session in data_sessions_:" + data_session);
        data_sessions_.at (data_session)->RemoveSession ();
        data_sessions_.erase (data_session);
    }

    return Status::OK;
}

// CollectGlobalStatisticsAggregated call. Collect Statistics request from core controller.
// It aggregates the statistics from several rounds.
Status LocalControlApplication::CollectGlobalStatisticsAggregated (ServerContext* context,
    const controllers_grpc_interface::ControlOperation* request,
    controllers_grpc_interface::StatsGlobalMap* reply)
{
    Logging::log_info ("LocalControlApplication: Received collect aggregated statistics "
                       "request from core controller");

    std::string rule = std::to_string (COLLECT_DETAILED_STATS) + "|"
        + std::to_string (COLLECT_GLOBAL_STATS) + "|";

    auto& stats_map = *reply->mutable_gl_stats ();

    for (int i = 0; i < COLLECT_ROUNDS; i++) {

        auto start = std::chrono::steady_clock::now ();

        // submit requests to each DataPlaneSession's submission_queue
        for (auto const& data_session : data_sessions_) {
            // put request on DataPlaneSession::submission_queue
            data_session.second->SubmitRule (rule);
        }

        std::list<std::string> sessions_to_delete;

        // collect requests from each DataPlaneSession's completion_queue
        for (auto const& data_session : data_sessions_) {

            // wait for request to be on DataPlaneSession::completion_queue
            std::unique_ptr<StageResponse> stats_ptr = data_session.second->GetResult ();

            // verify if pointer is valid
            if (stats_ptr != nullptr) {
                // convert StageResponse unique-ptr to StageResponseStatsKVS
                auto* response_ptr = dynamic_cast<StageResponseStat*> (stats_ptr.get ());

                if (response_ptr->get_total_rate () == -1) {
                    Logging::log_info (
                        "LocalControlApplication: CollectGlobalStatisticsAggregated ->"
                        "Connection error; disconnecting from instance-"
                        + data_session.first);
                    this->m_active_data_plane_sessions.fetch_sub (1);
                    sessions_to_delete.push_back (data_session.first);
                } else {
                    std::string name = data_session.first;

                    if (!stats_map.contains (name)) {
                        controllers_grpc_interface::StatsGlobal stats_global;
                        stats_global.set_m_metadata_total_rate (response_ptr->get_total_rate ());
                        stats_map[name] = stats_global;
                    } else {
                        auto stats_global = stats_map.at (name);
                        double previous_value = stats_global.m_metadata_total_rate ();
                        stats_global.set_m_metadata_total_rate (
                            (double)(response_ptr->get_total_rate () + previous_value * i)
                            / (i + 1));
                        stats_map[name] = stats_global;
                    }
                }

            } else {
                Logging::log_info ("LocalControlApplication: CollectGlobalStatisticsAggregated ->"
                                   "Connection error; disconnecting from instance-"
                    + data_session.first);
                this->m_active_data_plane_sessions.fetch_sub (1);
            }
        }

        for (auto const& data_session : sessions_to_delete) {
            controllers_grpc_interface::StatsGlobal stats_global;
            stats_global.set_m_metadata_total_rate (-1);
            stats_map[data_session] = stats_global;
            Logging::log_info (
                "LocalControlApplication: Deleting session in data_sessions_:" + data_session);
            data_sessions_.erase (data_session);
        }

        auto end = std::chrono::steady_clock::now ();

        if (i < 4) {
            std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time
                - std::chrono::duration_cast<std::chrono::microseconds> (end - start).count ()));
        }
    }

    return Status::OK;
}

////////////////////////////////////////////
//////////// Auxiliary Functions ///////////
////////////////////////////////////////////

//  initialize call. Initialize control application.
void LocalControlApplication::initialize ()
{ }

// handle_data_plane_sessions call. Processes pending data plane sessions.
void LocalControlApplication::handle_data_plane_sessions ()
{
    if (!pending_data_sessions_.empty ()) {
        Logging::log_debug ("LocalControlApplication: Data Plane Session Handshake");

        PStatus status = PStatus::Error ();

        pending_data_plane_sessions_lock_.lock ();
        auto handshake_session = std::move (pending_data_sessions_.front ());
        pending_data_sessions_.pop ();
        pending_data_plane_sessions_lock_.unlock ();

        m_pending_data_plane_sessions.fetch_sub (1);

        /*Start Session*/
        std::thread session_thread_t
            = std::thread (&HandshakeSession::StartSession, handshake_session.get ());
        session_thread_t.detach ();

        // invoke CallStageHandshake routine to acknowledge the stage's identifier
        //<stage_name, stage_env, stage_user>
        std::unique_ptr<StageInfo> stage_identifier
            = this->call_stage_handshake (handshake_session.get ());

        handshake_session->RemoveSession ();

        if (!stage_identifier->m_stage_name.empty ()) {
            // submit housekeeping rules to the data plane stage
            std::string stage_env
                = stage_identifier->m_stage_name + "+" + stage_identifier->m_stage_env;

            int installed_rules = this->submit_housekeeping_rules (stage_env);

            Logging::log_debug ("LocalControlApplication: installed rules ... ("
                + std::to_string (installed_rules) + ") in (" + stage_env + ")");

            if (installed_rules == housekeeping_rules_ptr_->size ()
                && mark_stage_ready (stage_env).isOk ()) {

                Logging::log_debug ("LocalControlApplication: Connecting Stage to Global ("
                    + stage_identifier->m_stage_name + ")");

                Status status = ConnectStageToGlobal (stage_identifier->m_stage_name,
                    stage_identifier->m_stage_env,
                    stage_identifier->m_stage_user);

                if (status.ok ()) {
                    m_active_data_plane_sessions.fetch_add (1);

                    Logging::log_debug ("DataPlaneSessionHandshake with DataPlaneStage-"
                        + stage_identifier->m_stage_name + " successfully established.");
                } else {
                    Logging::log_error ("DataPlaneSessionHandshake with DataPlaneStage-"
                        + stage_identifier->m_stage_name + " not established.");

                    std::this_thread::sleep_for (milliseconds (100));
                }

            } else {
                Logging::log_error ("DataPlaneSessionHandshake with DataPlaneStage-"
                    + stage_identifier->m_stage_name + " not established.");

                preparing_data_sessions_.at (stage_env)->RemoveSession ();
                preparing_data_sessions_.erase (stage_env);
            }
        }
    }
}

// call_stage_handshake call.  Submits STAGE_HANDSHAKE rule and housekeeping rules to data plane
// stage.
std::unique_ptr<StageInfo> LocalControlApplication::call_stage_handshake (
    HandshakeSession* handshake_session)
{
    // create STAGE_HANDSHAKE request
    std::string rule = std::to_string (STAGE_HANDSHAKE) + "|";
    // put request on DataPlaneSession::submission_queue

    handshake_session->SubmitRule (rule);

    // wait for request to be on DataPlaneSession::completion_queue
    std::unique_ptr<StageResponse> response_obj = handshake_session->GetResult ();
    // convert StageResponse to Handshake object

    auto* handshake_ptr = dynamic_cast<StageResponseHandshake*> (response_obj.get ());

    std::string stage_name_env
        = handshake_ptr->get_stage_name () + "+" + handshake_ptr->get_stage_env ();

    Logging::log_info ("LocalControlApplication: Stage Handshake with " + stage_name_env);
    // register instance index to stage_name_env
    std::unique_ptr<StageInfo> all_stage_info = std::make_unique<StageInfo> ();

    if (handshake_ptr != nullptr) {

        Logging::log_info ("LocalControlApplication: establishing UNIX connection with "
                           "data plane stage.");

        std::string socket_info;
        PStatus status = fill_socket_info (handshake_ptr, socket_info);

        Logging::log_info ("LocalControlApplication: StageHandshake <" + socket_info + ">");

        int port = -1;

        this->preparing_data_sessions_[stage_name_env]
            = std::make_unique<DataPlaneSession> (socket_info.c_str ());

        std::thread session_thread_t = std::thread (&DataPlaneSession::StartSession,
            (this->preparing_data_sessions_[stage_name_env]).get ());
        session_thread_t.detach ();

        /*Send info about the address and port to connect to*/
        rule = std::to_string (STAGE_HANDSHAKE_INFO) + "|" + socket_info + "|"
            + std::to_string (port) + "|";
        handshake_session->SubmitRule (rule);

        std::unique_ptr<StageResponse> response_obj = handshake_session->GetResult ();
        // convert StageResponse to Handshake object
        // auto* handshake_ptr = dynamic_cast<StageResponseACK*> (response_obj.get ());

        all_stage_info->m_stage_name = handshake_ptr->get_stage_name ();
        all_stage_info->m_stage_env = handshake_ptr->get_stage_env ();
        all_stage_info->m_stage_user = handshake_ptr->get_stage_user ();

        // Logging message
        Logging::log_info ("LocalControlApplication: StageHandshake <"
            + handshake_ptr->get_stage_name () + ", "
            + std::to_string (handshake_ptr->get_stage_pid ()) + ", "
            + std::to_string (handshake_ptr->get_stage_ppid ()) + ">");
    }

    // return const value of the stage identifier's name
    return all_stage_info;
}

// submit_housekeeping_rules call. Submits housekeeping rules to data plane stage.
int LocalControlApplication::submit_housekeeping_rules (const std::string& stage_name_env) const
{
    PStatus status = PStatus::Error ();
    int rule_counter = 0;
    int valid_housekeeping_rules = 0;

    // read rules from housekeeping_rules_ptr and submit to the SubmissionQueue
    for (const auto& value : *housekeeping_rules_ptr_) {

        // submit rule to the SubmissionQueue
        status = preparing_data_sessions_.at (stage_name_env)->SubmitRule (value);

        // update the counter of submitted rules
        if (status.isOk ()) {
            rule_counter++;
        }
    }

    // read responses from the CompletionQueue
    for (int i = 0; i < rule_counter; i++) {
        // get rules from CompletionQueue and cast them to a StageResponseACK object
        std::unique_ptr<StageResponse> response
            = preparing_data_sessions_.at (stage_name_env)->GetResult ();
        auto* ack_ptr = dynamic_cast<StageResponseACK*> (response.get ());

        // validate data plane stage response
        if (ack_ptr != nullptr) {
            if (ack_ptr->ACKValue () == static_cast<int> (AckCode::ok)) {
                valid_housekeeping_rules++;
            } else {
                return 0;
            }
        }
    }

    return valid_housekeeping_rules;
}

// mark_stage_ready call. Submits STAGE_READY to data plane stage.
PStatus LocalControlApplication::mark_stage_ready (const std::string& stage_name_env) const
{
    PStatus status = PStatus::Error ();

    std::string rule = std::to_string (STAGE_READY) + "|";

    status = preparing_data_sessions_.at (stage_name_env)->SubmitRule (rule);

    std::unique_ptr<StageResponse> response = nullptr;
    if (status.isOk ()) {
        std::unique_ptr<StageResponse> response
            = preparing_data_sessions_.at (stage_name_env)->GetResult ();
        auto* ack_ptr = dynamic_cast<StageResponseACK*> (response.get ());

        // validate data plane stage response
        if (ack_ptr != nullptr) {
            if (ack_ptr->ACKValue () == static_cast<int> (AckCode::ok)) {
                status = PStatus::OK ();
            }
        }
    }
    return status;
}

// LocalPassthru call. General function to submit rules to data plane stages.
Status LocalControlApplication::LocalPassthru (const std::string stage_name_env,
    const std::string rule)
{

    this->data_sessions_[stage_name_env]->SubmitRule (rule);

    std::unique_ptr<StageResponse> ack_ptr = this->data_sessions_[stage_name_env]->GetResult ();

    // verify if pointer is valid
    if (ack_ptr != nullptr) {
        // convert StageResponse unique-ptr to StageResponseStatsKVS
        auto* response_ptr = dynamic_cast<StageResponseACK*> (ack_ptr.get ());

        if (response_ptr->ACKValue () == 1) {
            return Status::OK;
        } else {
            return Status::CANCELLED;
        }
    }

    return Status::CANCELLED;
}

// fill_socket_info call: Defines a new individual socket for data plane stage.
PStatus LocalControlApplication::fill_socket_info (StageResponseHandshake* handshake_ptr,
    std::string& socket_info)
{
    std::stringstream stream;

    PStatus status = PStatus::OK ();

    stream << "/tmp/";
    if (!handshake_ptr->get_stage_name ().empty ()) {
        stream << handshake_ptr->get_stage_name () << "_";
    } else {
        status = PStatus::Error ();
        Logging::log_error (
            "LocalControlApplication: StageResponseHandshake stage_name field is empty.");
    }

    if (!handshake_ptr->get_stage_env ().empty ()) {
        stream << handshake_ptr->get_stage_env () << "_";
    } else {
        status = PStatus::Error ();
        Logging::log_error (
            "LocalControlApplication: StageResponseHandshake stage_env field is empty.");
    }

    if (handshake_ptr->get_stage_pid () > 0) {
        stream << handshake_ptr->get_stage_pid () << "_";
    } else {
        status = PStatus::Error ();
        Logging::log_error (
            "LocalControlApplication: StageResponseHandshake stage_pid field is empty.");
    }

    if (handshake_ptr->get_stage_ppid () > 0) {
        stream << handshake_ptr->get_stage_ppid ();
    } else {
        status = PStatus::Error ();
        Logging::log_error (
            "LocalControlApplication: StageResponseHandshake stage_ppid field is empty.");
    }

    stream << ".socket";

    socket_info = stream.str ();

    return status;
}

void LocalControlApplication::parse_rule (const std::string& rule,
    std::vector<std::string>* tokens,
    const char c)
{
    size_t start;
    size_t end = 0;

    while ((start = rule.find_first_not_of (c, end)) != std::string::npos) {
        end = rule.find (c, start);
        tokens->push_back (rule.substr (start, end - start));
    }
}

} // namespace cheferd
