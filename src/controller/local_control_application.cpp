/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/controller/local_control_application.hpp>
#include <shepherd/utils/rules_file_parser.hpp>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
}

namespace shepherd {

// LocalControlApplication default constructor.
LocalControlApplication::LocalControlApplication (
    const std::string& core_address,
    const std::string& local_address) :
    ControlApplication {},
    data_sessions_ {},
    stage_name_env_to_index_ {},
    index_to_pid_ {},
    index_to_stage_name_env_ {},
    operation_to_channel_object {},
    core_stub_(LocalToGlobal::NewStub(
        grpc::CreateChannel(core_address, grpc::InsecureChannelCredentials())
        ))
{
    Logging::log_info ("LocalControlApplication initialized.");
    initialize ();
    this->core_address = core_address;
    this->local_address = local_address;

    std::thread control_application_thread_t = std::thread (&LocalControlApplication::RunGlobalToLocalServer, this);
    control_application_thread_t.detach ();

    ConnectLocalToGlobal();
}

// LocalControlApplication parameterized constructor.
LocalControlApplication::LocalControlApplication (
    std::vector<std::string>* rules_ptr,
    const std::string& core_address,
    const std::string& local_address,
    const uint64_t& cycle_sleep_time) :
    ControlApplication {  rules_ptr, cycle_sleep_time },
    data_sessions_ {},
    stage_name_env_to_index_ {},
    index_to_pid_ {},
    index_to_stage_name_env_ {},
    operation_to_channel_object {},
    core_stub_(LocalToGlobal::NewStub(
        grpc::CreateChannel(core_address, grpc::InsecureChannelCredentials())
            ))
{
    Logging::log_info ("LocalControlApplication parameterized constructor.");

    initialize ();
    this->core_address = core_address;
    this->local_address = local_address;


    std::thread control_application_thread_t = std::thread (&LocalControlApplication::RunGlobalToLocalServer, this);
    control_application_thread_t.detach ();

    ConnectLocalToGlobal();
}

//    LocalControlApplication default destructor.
LocalControlApplication::~LocalControlApplication ()
{
    std::cout << "LocalControlApplication: exiting ...\n";
}

void LocalControlApplication::RunGlobalToLocalServer() {

    //int base_address = 50051;
    //std::string user_address("0.0.0.0:" + std::to_string(base_address + local_id));

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(local_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with clients.
    // In this case it corresponds to an *synchronous* service.
    builder.RegisterService(this);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << local_address << std::endl;

    // Wait for the server to shutdown.
    // Note that some other thread must be responsible for shutting down the server for this call to ever return.
    server->Wait();
}

Status LocalControlApplication::ConnectLocalToGlobal() {
    // Data we are sending to the server.
    ConnectRequest request;
    request.set_user_address(local_address);

    // Container for the data we expect from the server.
    ConnectReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = core_stub_->ConnectLocalToGlobal(&context, request, &reply);

    std::cout << "Connect request sent to global controller" << std::endl;

    if (status.ok()){
        std::cout << reply.message() << std::endl;
    }
    else {
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
        std::cout << "RPC failed" << std::endl;
    }

    return status;
}

Status LocalControlApplication::ConnectStageToGlobal(const std::string& stage_name, const std::string& stage_env, const std::string& stage_user){
    // Data we are sending to the server.
    ConnectRequestStage request;
    request.set_local_address(local_address);
    request.set_stage_name(stage_name);
    request.set_stage_env(stage_env);
    request.set_stage_user(stage_user);

    // Container for the data we expect from the server.
    ConnectReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = core_stub_->ConnectStageToGlobal(&context, request, &reply);

    std::cout << "Connect stage request sent to global controller" << std::endl;

    if (status.ok()){
        std::cout << reply.message() << std::endl;
    }
    else {
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
        std::cout << "RPC failed" << std::endl;
    }

    return status;
}


//    Initialize call. (...)
void LocalControlApplication::initialize ()
{

}


//    RegisterDataPlaneSession call. (...)
DataPlaneSession* LocalControlApplication::register_stage_session (int index)
{
    Logging::log_debug ("RegisterDataPlaneSession -- TensorFlow-" + std::to_string (index));

    data_sessions_.push_back(std::make_unique<DataPlaneSession> ());

    this->m_pending_data_plane_sessions.fetch_add (1);

    return data_sessions_[index].get ();
}

// stage_handshake call. (...)
PStatus LocalControlApplication::stage_handshake (int index)
{
    Logging::log_debug (
        "LocalControlApplication: Data Plane Session Handshake (" + std::to_string (index) + ")");
    PStatus status = PStatus::Error ();

    // invoke CallStageHandshake routine to acknowledge the stage's identifier
    //<stage_name, stage_env, stage_user>
    std::tuple<const std::string, const std::string, const std::string> stage_identifier = this->call_stage_handshake (index);

    status = PStatus::OK();
    if (!std::get<0>(stage_identifier).empty ()) {
        // submit housekeeping rules to the data plane stage
        int installed_rules = this->submit_housekeeping_rules (index);
        Logging::log_debug (
            "installed rules ... (" + std::to_string (installed_rules) + ")");

        if (installed_rules > 0) {
            // mark data plane stage ready
            status = this->mark_data_plane_stage_ready (index);
        }
    }

    Logging::log_debug (
        "LocalControlApplication: Connecting Stage to Global (" + std::to_string (index) + ")");

    if (status.isOk()) {
        ConnectStageToGlobal(std::get<0>(stage_identifier), std::get<1>(stage_identifier), std::get<2>(stage_identifier));
    }

    return status;
}

// mark_data_plane_stage_ready call. (...)
PStatus LocalControlApplication::mark_data_plane_stage_ready (const int& index)
{
    Logging::log_debug ("LocalControlApplication: mark stage ready (" + std::to_string (index) + ")");
    PStatus status = PStatus::Error ();

    std::string rule = std::to_string (STAGE_READY) + "|";
    // put request on DataPlaneSession::submission_queue
    data_sessions_[index]->SubmitRule (rule);

    // get rules from CompletionQueue and cast them to a StageResponseACK object
    std::unique_ptr<StageResponse> resp_t = data_sessions_[index]->GetResult ();
    auto* ack_ptr_t = dynamic_cast<StageResponseACK*> (resp_t.get ());

    if (ack_ptr_t != nullptr) {
        if (ack_ptr_t->ACKValue () == static_cast<int> (AckCode::ok)) {
            status = PStatus::OK ();
        }
    }

    return status;
}

// operator call. (...)
void LocalControlApplication::operator () ()
{
    this->execute_feedback_loop ();
}

// execute_feedback_loop call. (...)
void LocalControlApplication::execute_feedback_loop ()
{
    Logging::log_debug ("LocalControlApplication::ExecuteFeedbackLoop");
    PStatus status = PStatus::Error ();
    int index = 0;

    // wait for a data plane stage to connect
    while (this->m_pending_data_plane_sessions.load () == 0) {
        std::this_thread::sleep_for (milliseconds (100));
    }

    // while existing data plane stage connections (active or pending), execute the feedback loop
    while ((this->m_pending_data_plane_sessions.load () + this->m_active_data_plane_sessions.load ()) > 0) {
        // if exists pending sessions, perform the Session Handshake
        while (this->m_pending_data_plane_sessions.load () > 0) {
            // execute session handshake
            status = this->stage_handshake (index);

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

        // 1st phase: collect statistics from existing data plane stages
        //const std::array<StatsTFControlApplication2, TENSORFLOW_STATISTICS>& tf_stats
          //  = this->collect_statistics (this->m_active_data_plane_sessions.load (), 0);

        // 2nd phase: compute statistics and enforce rules
        // Compute (tf_stats, active_data_plane_sessions.load(), index - active_data_plane_sessions)
        //this->compute (tf_stats, this->m_active_data_plane_sessions.load (), 0);

        // 3rd phase: sleep for the next feedback loop cycle
        //this->sleep ();
    }

    // log message and end control loop
    Logging::log_info ("Exiting. No active connections.");
    _exit (EXIT_SUCCESS);
}

// call_stage_handshake call. (...)
std::tuple<const std::string, const std::string, const std::string> LocalControlApplication::call_stage_handshake (const int &index)
{
    // create STAGE_HANDSHAKE request
    std::string rule = std::to_string (STAGE_HANDSHAKE) + "|";
    // put request on DataPlaneSession::submission_queue
    data_sessions_[index]->SubmitRule (rule);

    // wait for request to be on DataPlaneSession::completion_queue
    std::unique_ptr<StageResponse> response_obj = data_sessions_[index]->GetResult ();
    // convert StageResponse to Handshake object
    auto* handshake_ptr = dynamic_cast<StageResponseHandshake*> (response_obj.get ());

    // register instance PID
    if (handshake_ptr != nullptr) {
        stage_name_env_to_index_.emplace(handshake_ptr->get_stage_name() + "+" + handshake_ptr->get_stage_env(), index);
        index_to_stage_name_env_.emplace(index, std::make_pair (handshake_ptr->get_stage_name(), handshake_ptr->get_stage_env()));
        index_to_pid_.emplace(index, handshake_ptr->get_stage_pid ());
    }

    // Logging message
    Logging::log_info ("StageHandshake <" +
                       handshake_ptr->get_stage_name () + ", " +
                       std::to_string (handshake_ptr->get_stage_pid ()) + ", " +
                       std::to_string (handshake_ptr->get_stage_ppid ()) + ">");

    // return const value of the stage identifier's name
    return std::make_tuple(handshake_ptr->get_stage_name(), handshake_ptr->get_stage_env(), handshake_ptr->get_stage_user());
}

// submit_housekeeping_rules call. (...)
int LocalControlApplication::submit_housekeeping_rules (const int &index) const
{
    PStatus status = PStatus::Error ();
    int rule_counter = 0;
    int valid_housekeeping_rules = 0;


    // read rules from housekeeping_rules_ptr and submit to the SubmissionQueue
    for (const auto& value : *housekeeping_rules_ptr_) {
        // submit rule to the SubmissionQueue
        status = data_sessions_[index]->SubmitRule (value);

        // update the counter of submitted rules
        if (status.isOk ()) {
            rule_counter++;
        }
    }

    // read responses from the CompletionQueue
    for (int i = 0; i < rule_counter; i++) {
        // get rules from CompletionQueue and cast them to a StageResponseACK object
        std::unique_ptr<StageResponse> response = data_sessions_[index]->GetResult ();
        auto* ack_ptr = dynamic_cast<StageResponseACK*> (response.get ());

        // validate data plane stage response
        if (ack_ptr != nullptr) {
            if (ack_ptr->ACKValue () == static_cast<int> (AckCode::ok)) {
                valid_housekeeping_rules++;
            }
        }
    }

    return valid_housekeeping_rules;
}

//    CollectStatistics call. (...)
std::unique_ptr<StageResponse> LocalControlApplication::collect_statistics ()
{
    //PidIOStats2 pid_io_stats = this->collect_pid_stats (i);
    // FIXME: ------------------------------------------------
    //        PidIOStats pid_io_stats {};
    //        pid_io_stats.m_pid = m_instance_pid [start_index];
    //        pid_io_stats.m_read_thr = 0;
    //        pid_io_stats.m_write_thr = 0;
    // FIXME: îîîîîîîîî


    // update StatsTFControlApplication (read/write throughput of a given PID) for index i
    //tf_stats[i].m_pid_read_bandwidth = pid_io_stats.m_read_thr;
    //tf_stats[i].m_pid_write_bandwidth = pid_io_stats.m_write_thr;

    // update aggregated I/O bandwidth of processes
    //tf_stats[TENSORFLOW_STATISTICS - 1].m_pid_read_bandwidth += pid_io_stats.m_read_thr;
    //tf_stats[TENSORFLOW_STATISTICS - 1].m_pid_write_bandwidth += pid_io_stats.m_write_thr;

    return nullptr;
}



//    Compute call. (...)
void LocalControlApplication::compute (const std::unique_ptr<StageResponse>& statistics_ptr)
{
    Logging::log_debug ("LocalControlApplication::Compute");
}

// sleep call. (...)
void LocalControlApplication::sleep ()
{
    std::this_thread::sleep_for (microseconds (this->m_feedback_loop_sleep_time));
}

// parse_io_stats call. Parses both read_bytes and write_bytes entries of a /proc/<pid>/io call.
void parse_io_stats1 (const std::string& io_stats, long* read_bytes, long* write_bytes)
{
    std::regex words_regex ("[^\\s. ]+");
    auto words_begin = std::sregex_iterator (io_stats.begin (), io_stats.end (), words_regex);
    auto words_end = std::sregex_iterator ();
    int stat_entry = 0;

    for (std::sregex_iterator iterator = words_begin; iterator != words_end; ++iterator) {
        if (stat_entry == 9) {
            *read_bytes = ::atol ((*iterator).str ().c_str ());
        } else if (stat_entry == 11) {
            *write_bytes = ::atol ((*iterator).str ().c_str ());
        }
        stat_entry++;
    }
}

// collect_pid_stats call. Collect I/O statistics of a given process.
PidIOStats2 LocalControlApplication::collect_pid_stats (const int &index)
{
    //std::string path = "/proc/" + std::to_string (this->m_instance_pid[index]) + "/io";
    // std::string path = "/home/gsd/db/shepherd/tests/4100";
    std::string path = "/Users/marianamiranda/Desktop/PhD/code/paddl/shepherd/tests/4100";
    // std::string path = "/home/acb11912na/db/shepherd/tests/4100";

    long read_bytes, write_bytes;
    char io_stats[1024];

    // open /proc/<pid>/io file
    int fd = ::open (path.c_str (), O_RDONLY);

    // verify open return value
    if (fd == -1) {
        Logging::log_error ("CollectPidStats: error while opening " + path);
    }

    // read I/O stats from /proc/<pid>/io
    ssize_t return_value = ::read (fd, &io_stats, 1024);

    // verify read return value
    if (return_value <= 0) {
        Logging::log_error ("CollectPidStats: could not read from " + path);
    }

    // parse I/O stats
    parse_io_stats1 (std::string (io_stats), &read_bytes, &write_bytes);

    // create PidIOStats object
    PidIOStats2 pid_io_stats { this->index_to_pid_[index],
                               static_cast<double>(read_bytes - this->m_previous_read_bytes_pid[index])
                               / (static_cast<double>(this->m_feedback_loop_sleep_time) / 1000 / 1000),
                               static_cast <double>(write_bytes - this->m_previous_write_bytes_pid[index])
                               / (static_cast <double>(this->m_feedback_loop_sleep_time) / 1000 / 1000) };

    // update previous_{read/write}_bytes
    this->m_previous_read_bytes_pid[index] = read_bytes;
    this->m_previous_write_bytes_pid[index] = write_bytes;

    // close file descriptor
    int close_return_value = ::close (fd);

    // verify close return value
    if (close_return_value == -1) {
        Logging::log_error ("CollectPidStats: error on closing " + path);
    }

    return pid_io_stats;
}


void LocalControlApplication::parse_rule(const std::string& rule, std::vector<std::string>* tokens, const char c)
{
    size_t start;
    size_t end = 0;

    while ((start = rule.find_first_not_of (c, end)) != std::string::npos) {
        end = rule.find (c, start);
        tokens->push_back (rule.substr (start, end - start));
    }
}


/*Requests from the CORE controller*/
Status LocalControlApplication::LocalHandshake(ServerContext* context, const controllers_grpc_interface::LocalSimplifiedHandshakeRaw* request,
    controllers_grpc_interface::ACK* reply)  {
    std::cout <<"Local Handshake from core controller" << std::endl;

    for(auto & rule : request->rules()){

        std::cout << "Received local handshake from core controller: " << rule << std::endl;
        housekeeping_rules_ptr_->push_back (rule);

        std::vector<std::string> rule_tokens {};
        parse_rule(rule, &rule_tokens, '|');

        if (rule_tokens[2].compare("create_object") == 0) {
            auto channel_object = std::make_pair(std::stoi(rule_tokens[3]), std::stoi(rule_tokens[4]));

            auto channels_objects = operation_to_channel_object.find(rule_tokens[6]);

            if ( channels_objects == operation_to_channel_object.end() ) {

                std::vector<std::pair<int, int>> new_channels_objects;
                new_channels_objects.push_back(channel_object);
                operation_to_channel_object.emplace(rule_tokens[6],new_channels_objects);
            } else {
                channels_objects->second.push_back (channel_object);
            }
        }
    }

    reply->set_m_message(1);
    return Status::OK;
}


Status LocalControlApplication::StageHandshake(ServerContext* context, const controllers_grpc_interface::ControlOperation* request,
    controllers_grpc_interface::StageSimplifiedHandshakeRaw* reply)  {
    std::cout <<"Stage Handshake at core controller" << std::endl;
    reply->set_m_stage_env("tensorflow-1");
    //std::cout << "Received request from local controller at address:"<< request->user_address() << std::endl;
    //EnqueueAddressInRequestQueue(request);
    //reply->set_message(prefix + request->user_address());
    return Status::OK;
}


Status LocalControlApplication::MarkStageReady(ServerContext* context, const controllers_grpc_interface::StageReadyRaw* request,
    controllers_grpc_interface::ACK* reply){
    std::cout <<"MarkStageReadyPhase2 from core controller" << std::endl;
    reply->set_m_message(1);
    return Status::OK;
}


Status LocalControlApplication::CreateHouseKeepingRuleChannel(ServerContext* context, const controllers_grpc_interface::HousekeepingCreateChannelString* request,
    controllers_grpc_interface::ACK* reply){
    std::cout <<"CreateHouseKeepingRuleChannelPhase2 from core controller" << std::endl;

    Status status = LocalPassthru(request->m_stage_name(), request->m_stage_env() , request->m_rule());

    if (status.ok()) {
        reply->set_m_message(1);
    }
    else {
        reply->set_m_message(0);
    }

    return status;
}

Status LocalControlApplication::CreateHouseKeepingRuleObject(ServerContext* context, const controllers_grpc_interface::HousekeepingCreateObjectString* request,
    controllers_grpc_interface::ACK* reply){
    std::cout <<"CreateHouseKeepingRuleObject from core controller" << std::endl;

    Status status = LocalPassthru(request->m_stage_name(), request->m_stage_env() , request->m_rule());

    if (status.ok()) {
        reply->set_m_message(1);
    }
    else {
        reply->set_m_message(0);
    }

    return status;
}

Status LocalControlApplication::ExecuteHousekeepingRules(ServerContext* context, const controllers_grpc_interface::Execute* request,
    controllers_grpc_interface::ACK* reply){
    std::cout <<"ExecuteHousekeepingRules from core controller" << std::endl;

    Status status = LocalPassthru(request->m_stage_name(), request->m_stage_env() , "");

    if (status.ok()) {
        reply->set_m_message(1);
    }
    else {
        reply->set_m_message(0);
    }

    return status;
}

Status LocalControlApplication::CreateEnforcementRule(ServerContext* context, const controllers_grpc_interface::EnforcementRuleString* request,
    controllers_grpc_interface::ACK* reply){
    std::cout <<"CreateEnforcementRule from core controller" << std::endl;

    Status status = Status::OK;

    for(auto & env_rate : request->env_rates()){

        /*TO-DO: Select channel and enforcement_object based on operaiton */



        auto existing_channels = operation_to_channel_object.find (request->m_operation());

        if (existing_channels == operation_to_channel_object.end ()) {

        }
        else {

            int total_channels = existing_channels->second.size();
            int limit_per_channel = 0;
            if (total_channels > 0) {
                limit_per_channel = std::floor (env_rate.second / total_channels);
            }

            for (auto & channel_objects: existing_channels->second){

                int channel_id = channel_objects.first;
                int enforcement_object_id = channel_objects.second;

                std::string enforcement_rule = std::to_string (CREATE_ENF_RULE) + "|"
                    + std::to_string (request->m_rule_id()) + "|"  // rule-id
                    + std::to_string (channel_id) + "|"
                    + std::to_string(enforcement_object_id) + "|"
                    + "drl" + "|"
                    + "rate" + "|"
                    + std::to_string (limit_per_channel);

                status = LocalPassthru(request->m_stage_name(), std::to_string (env_rate.first) , enforcement_rule);

                if (status.ok()) {
                    reply->set_m_message(1);
                }
                else {
                    reply->set_m_message(0);
                    return status;
                }
            }
        }
    }

    return status;
}

Status LocalControlApplication::LocalPassthru (const std::string stage_name, const std::string stage_env, const std::string rule){


    int index = stage_name_env_to_index_.at(stage_name + "+" + stage_env);

    this->data_sessions_[index]->SubmitRule (rule);

    std::unique_ptr<StageResponse> ack_ptr = this->data_sessions_[index]->GetResult ();


    // verify if pointer is valid
    if (ack_ptr != nullptr) {
        // convert StageResponse unique-ptr to StageResponseStatsKVS
        auto* response_ptr = dynamic_cast<StageResponseACK*> (ack_ptr.get ());

        Logging::log_debug ("ACK response :: " + std::to_string (response_ptr->ResponseType ())
            + " -- " + response_ptr->toString ());

        if (response_ptr->ACKValue () == 1) {
            return Status::OK;
        } else {
            return Status::CANCELLED;
        }
    }

    return Status::CANCELLED;
}

Status LocalControlApplication::CollectGlobalStatistics (ServerContext* context, const controllers_grpc_interface::ControlOperation* request, controllers_grpc_interface::StatsGlobalMap* reply) {
    std::cout <<"CollectGlobalStatistics from core controller" << std::endl;

    int start_index = 0;
    const int active_sessions = this->m_active_data_plane_sessions.load ();

    std::string rule = std::to_string (COLLECT_GLOBAL_STATS) + "|";

    // submit requests to each DataPlaneSession's submission_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // put request on DataPlaneSession::submission_queue
        data_sessions_[i]->SubmitRule (rule);
    }



    // collect requests from each DataPlaneSession's completion_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // wait for request to be on DataPlaneSession::completion_queue
        std::unique_ptr<StageResponse> stats_ptr = data_sessions_[i]->GetResult ();

        // verify if pointer is valid
        if (stats_ptr != nullptr) {
            // convert StageResponse unique-ptr to StageResponseStatsKVS
            auto* response_ptr = dynamic_cast<StageResponseStatsGlobal*> (stats_ptr.get ());

            if (response_ptr->get_read_rate () == -1) {
                Logging::log_error ("collect_statistics_global: Connection error; disconnecting from instance-" +
                    std::to_string (i));
                this->m_active_data_plane_sessions.fetch_sub (1);
            } else {



                controllers_grpc_interface::StatsGlobal stat_temp = *reply->add_stats();

                auto stage_name_env = index_to_stage_name_env_.at(i);

                stat_temp.set_stage_name (stage_name_env.first);
                stat_temp.set_stage_env(stage_name_env.second);
                stat_temp.set_m_read_rate(response_ptr->get_read_rate ());
                stat_temp.set_m_write_rate(response_ptr->get_write_rate ());
                stat_temp.set_m_open_rate (response_ptr->get_open_rate());
                stat_temp.set_m_close_rate (response_ptr->get_close_rate());
                stat_temp.set_m_getattr_rate (response_ptr->get_getattr_rate());
                stat_temp.set_m_metadata_total_rate (response_ptr->get_metadata_total_rate());
            }
        }
    }

    return Status::OK;
}


Status LocalControlApplication::CollectEntityStatistics (ServerContext* context, const controllers_grpc_interface::ControlOperation* request, controllers_grpc_interface::StatsEntityMap* reply) {

    Logging::log_debug ("CollectEntityStatistics from core controller");

    int start_index = 0;
    int active_sessions = this->m_active_data_plane_sessions.load ();

    std::string rule = std::to_string (COLLECT_ENTITY_STATS) + "|";

    // submit requests to each DataPlaneSession's submission_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // put request on DataPlaneSession::submission_queue
        data_sessions_[i]->SubmitRule (rule);
    }

    auto& stats_map = *reply->mutable_stats();

    // collect requests from each DataPlaneSession's completion_queue
    for (int i = start_index; i < (active_sessions + start_index); i++) {
        // wait for request to be on DataPlaneSession::completion_queue
       std::unique_ptr<StageResponse> stats_ptr = data_sessions_[i]->GetResult ();

        // verify if pointer is valid
        if (stats_ptr != nullptr) {
            // convert StageResponse unique-ptr to StageResponseStatsEntity
            auto* response_ptr = dynamic_cast<StageResponseStatsEntity*> (stats_ptr.get ());

            if (response_ptr == nullptr) {
                Logging::log_error ("collect_statistics_entity: Connection error; disconnecting from instance-" +
                    std::to_string (i));
                this->m_active_data_plane_sessions.fetch_sub (1);
            } else {

                std::string name = index_to_stage_name_env_[i].first + "+" +  index_to_stage_name_env_[i].second;

                controllers_grpc_interface::StatsEntity stats_entity;
                auto& stats_entity_temp = *stats_entity.mutable_ent_stats();

                std::unordered_map<std::basic_string<char>, double>* rates = response_ptr->entity_rates.get();


                for (auto& key_value: *rates){
                    stats_entity_temp[key_value.first] = key_value.second;

                }

                stats_map[name] = stats_entity;

            }

        }
    }

    return Status::OK;
}

} // namespace shepherd
