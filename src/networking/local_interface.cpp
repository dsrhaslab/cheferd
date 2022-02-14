/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/networking/local_interface.hpp>
#include <shepherd/utils/rules_file_parser.hpp>

/**
 * TODO:
 *  - implement mark data plane stage ready
 *  - adjust rule-parser
 *  - adjust conversion of rules from string to structures
 */

namespace shepherd {

// LocalInterface default constructor.
//LocalInterface::LocalInterface () = default;

LocalInterface::LocalInterface(const std::string& user_address)
    : stub_(GlobalToLocal::NewStub(
        grpc::CreateChannel(user_address, grpc::InsecureChannelCredentials()))) {
}

// LocalInterface default destructor.
LocalInterface::~LocalInterface () = default;

// stage_handshake call. Submit a handshake request to identify the Data Plane Stage that has
// established communication.
PStatus LocalInterface::local_handshake (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    // pre-send phase
    // prepare ControlSend object

    controllers_grpc_interface::ACK reply;

    controllers_grpc_interface::ControlOperation operation1;

    operation1.set_m_operation_id(-1);
    operation1.set_m_operation_type(LOCAL_HANDSHAKE);
    operation1.set_m_size(sizeof (struct StageSimplifiedHandshakeRaw));

    // parsing phase
    controllers_grpc_interface::LocalSimplifiedHandshakeRaw housekeeping_rules;
    fill_housekeeping_rules_grpc(&housekeeping_rules, rule);

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->LocalHandshake(&context, housekeeping_rules, &reply);


    if(!status.ok()){
        Logging::log_error ("stage_handshake: Error while handshake control operation (" + status.error_message() + ").");
        return PStatus::Error();
    } else {
        std::cout << "stage_handshake: control operation submitted\n";

        std::cout << "LocalInterface: ControlOperation: ";
        std::cout << operation->m_operation_id << ", ";
        std::cout << operation->m_operation_type << ", ";
        std::cout << operation->m_operation_subtype << ", ";
        std::cout << operation->m_size << "\n";

        response.m_message = reply.m_message ();

        return PStatus::OK ();
    }

}


// stage_handshake call. Submit a handshake request to identify the Data Plane Stage that has
// established communication.
PStatus LocalInterface::stage_handshake (const std::string& user_address,
                                         ControlOperation* operation,
                                         StageSimplifiedHandshakeRaw& stage_info_obj)
{
    // pre-send phase
    // prepare ControlSend object

    operation->m_operation_id = -1;
    operation->m_operation_type = STAGE_HANDSHAKE;
    operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);


    controllers_grpc_interface::StageSimplifiedHandshakeRaw reply;

    controllers_grpc_interface::ControlOperation operation1;

    operation1.set_m_operation_id(-1);
    operation1.set_m_operation_type(STAGE_HANDSHAKE);
    operation1.set_m_size(sizeof (struct StageSimplifiedHandshakeRaw));

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StageHandshake(&context, operation1, &reply);


    if(!status.ok()){
        Logging::log_error ("stage_handshake: Error while handshake control operation (" + status.error_message() + ").");
        return PStatus::Error();
    } else {
        std::cout << "stage_handshake: control operation submitted\n";

        std::cout << "LocalInterface: ControlOperation: ";
        std::cout << operation->m_operation_id << ", ";
        std::cout << operation->m_operation_type << ", ";
        std::cout << operation->m_operation_subtype << ", ";
        std::cout << operation->m_size << "\n";


        strcpy(stage_info_obj.m_stage_name, reply.m_stage_name().c_str());
        strcpy(stage_info_obj.m_stage_env, reply.m_stage_env().c_str());
        stage_info_obj.m_pid = reply.m_pid();
        stage_info_obj.m_ppid = reply.m_ppid();

        // debug message
        std::stringstream stream;
        stream << "Serialize::StageSimplifiedHandshakeRaw\n";
        stream << "   name\t\t: " << stage_info_obj.m_stage_name;
        stream << " (" << sizeof (stage_info_obj.m_stage_name) << ")\n";
        stream << "   env\t\t: " << stage_info_obj.m_stage_env;
        stream << " (" << sizeof (stage_info_obj.m_stage_env) << ")\n";
        stream << "   pid\t\t: " << stage_info_obj.m_pid << "\n";
        stream << "   ppid\t\t: " << stage_info_obj.m_ppid << "\n";
        stream << "Size of struct: " << sizeof (StageSimplifiedHandshakeRaw) << "\n";
        Logging::log_debug (stream.str());

        return PStatus::OK ();
    }

}

PStatus LocalInterface::mark_stage_ready (const std::string& user_address,
    ControlOperation* operation,
    StageReadyRaw& stage_ready_obj,
    ACK& response)
{

    controllers_grpc_interface::ACK reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // send phase
    stage_ready_obj.m_mark_stage = true;

    controllers_grpc_interface::StageReadyRaw stage_ready_raw;
    stage_ready_raw.set_m_mark_stage(true);

    Status status = stub_->MarkStageReady(&context, stage_ready_raw, &reply);

    response.m_message = reply.m_message ();

    if (!status.ok()) {
        Logging::log_error ("mark_stage_ready (channel): Error while writing stage ready (" + status.error_message() + ").");
        return PStatus::Error();
    } else if (reply.m_message () == static_cast<int>(AckCode::ok)) {
        Logging::log_debug ("mark_stage_ready: ACK message received (" + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error();
    }

}

// create_housekeeping_rule call. (...)
PStatus LocalInterface::create_housekeeping_rule (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    Logging::log_debug ("create_housekeeping_rule: " + rule);

    // parsing phase
    std::vector<std::string> rule_tokens {};
    this->parse_rule (rule, &rule_tokens);

    if (rule_tokens.empty ()) {
        Logging::log_error ("create_housekeeping_rule: empty rule.");
        return PStatus::Error ();
    }

    for (int i = 0; i < rule_tokens.size(); i++) {
        std::cout << i << " -- " << rule_tokens[i] << "\n";
    }

    controllers_grpc_interface::ACK reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    switch (operation->m_operation_subtype) {
        case HSK_CREATE_CHANNEL: {
            std::cout << "... HSK_CREATE_CHANNEL ...\n";

            // prepare HousekeepingCreateChannelRaw object to be sent
            controllers_grpc_interface::HousekeepingCreateChannelString create_channel_rule;
            //this->fill_create_channel_rule_grpc (&create_channel_rule, rule_tokens);

            create_channel_rule.set_m_stage_name("tensor");
            create_channel_rule.set_m_stage_env("1");
            create_channel_rule.set_m_rule(rule);

            // send phase
            Status status = stub_->CreateHouseKeepingRuleChannel(&context, create_channel_rule, &reply);

            response.m_message = reply.m_message();

            if (!status.ok()) {
                Logging::log_error ("create_housekeeping_rule (channel): Error while writing housekeeping rule (" + status.error_message() + ").");
                return PStatus::Error();
            } else if (reply.m_message () == static_cast<int>(AckCode::ok)) {
                Logging::log_debug ("create_housekeeping_rule: ACK message received (" + std::to_string (response.m_message) + ").");
            } else {
                return PStatus::Error();
            }

            break;
        }

        case HSK_CREATE_OBJECT: {
            std::cout << "... HSK_CREATE_OBJECT ...\n";

            // prepare HousekeepingCreateObjectRaw object to be sent
            controllers_grpc_interface::HousekeepingCreateObjectString create_object_rule;
            //this->fill_create_object_rule_grpc (&create_object_rule, rule_tokens);
            create_object_rule.set_m_stage_name("tensor");
            create_object_rule.set_m_stage_env("1");
            create_object_rule.set_m_rule(rule);

            // send phase
            Status status = stub_->CreateHouseKeepingRuleObject(&context, create_object_rule, &reply);


            response.m_message = reply.m_message();

            if (!status.ok()) {
                Logging::log_error ("create_housekeeping_rule (object): Error while writing housekeeping rule (" + status.error_message() + ").");
                return PStatus::Error();
            } else if (reply.m_message () == static_cast<int>(AckCode::ok)) {
                Logging::log_debug ("create_housekeeping_rule: ACK message received (" + std::to_string (response.m_message) + ").");
            } else {
                return PStatus::Error();
            }

            break;
        }

        default:
            Logging::log_error ("After parsing -- other rule");
            return PStatus::Error ();
    }

    return PStatus::OK ();
}

// missing tests
// ExecuteHousekeepingRules call. (...)
PStatus LocalInterface::ExecuteHousekeepingRules (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    Logging::log_debug ("ExecuteHousekeepingRules:: " + rule);


    controllers_grpc_interface::ACK reply;
    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // send phase
    // prepare Execute object to be sent
    controllers_grpc_interface::Execute execute_obj;
    execute_obj.set_m_stage_name("tensor");
    execute_obj.set_m_stage_env("1");
    execute_obj.set_execute_all(true);

    // write Execute object through user_address
    Status status = stub_->ExecuteHousekeepingRules(&context, execute_obj, &reply);

    response.m_message = reply.m_message ();
    if (!status.ok()) {
        Logging::log_error ("execute_housekeeping_rules: Error while writing Execute object (" + status.error_message() + ").");
        return PStatus::Error();
    } else if (reply.m_message () == static_cast<int>(AckCode::ok)) {
        Logging::log_debug ("execute_housekeeping_rules: ACK message received (" + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error();
    }
}

// Missing actual implementation
// CreateDifferentiationRule call. (...)
PStatus LocalInterface::CreateDifferentiationRule (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    return PStatus::NotSupported ();
}

// missing tests
// create_enforcement_rule call. (...)
PStatus LocalInterface::create_enforcement_rule (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    // validate if logging is enabled and log debug message
    if (Logging::is_debug_enabled ()) {
        Logging::log_debug ("create_enforcement_rule: " + rule);
    }

    // parsing phase
    std::vector<std::string> rule_tokens {};
    this->parse_rule (rule, &rule_tokens);

    if (rule_tokens.empty ()) {
        Logging::log_error ("create_enforcement_rule: empty rule.");
        return PStatus::Error ();
    }

    controllers_grpc_interface::ACK reply;
    ClientContext context;

    controllers_grpc_interface::EnforcementRuleString create_enforcement_rule;

    create_enforcement_rule.set_m_rule_id(std::stoll (rule_tokens[1]));
    create_enforcement_rule.set_m_stage_name(rule_tokens[2]);
    create_enforcement_rule.set_m_operation(rule_tokens[3]);

    auto& rules_map = *create_enforcement_rule.mutable_env_rates();

    size_t start;
    size_t end = 0;

    while ((start = rule_tokens[4].find_first_not_of ('*', end)) != std::string::npos) {
        end = rule_tokens[4].find ('*', start);

        std::string token_rule = rule_tokens[4].substr (start, end - start);

        size_t start2;
        size_t end2 = 0;

        std::vector<std::string> tokens = {};
        while ((start2 = token_rule.find_first_not_of (':', end2)) != std::string::npos) {
            end2 = token_rule.find (':', start2);
            tokens.push_back(token_rule.substr (start2, end2 - start2));
        }

        auto env = std::stoll(tokens[0]);
        rules_map[env] = std::stoll(tokens[1]);
    }

        // write EnforcementRule object through user_address
    Status status = stub_->CreateEnforcementRule(&context, create_enforcement_rule, &reply);

    response.m_message = reply.m_message ();
    if (!status.ok()) {
        Logging::log_error ("create_enforcement_rule: Error while writing enforcement rule object to the local controler (" + status.error_message() + ").");
        return PStatus::Error();
    } else if (reply.m_message () == static_cast<int>(AckCode::ok)) {
        Logging::log_debug ("create_enforcement_rule: ACK message received (" + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error();
    }
}

// missing actual implementation (this is a placeholder)
// RemoveRule call. (...)
PStatus
LocalInterface::RemoveRule (const std::string& user_address, ControlOperation* operation, int rule_id, ACK& response)
{
    PStatus status = PStatus::Error ();
    // Pre-send Phase
    // Prepare ControlSend object
    operation->m_operation_id = -1;
    operation->m_size = sizeof (struct ControlOperation);

    ssize_t return_value = NULL;
        //::write (user_address, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error ("remove_rule: Error while writing control operation (" + std::to_string (return_value) + ").");
        return PStatus::Error();
    }

    // Send Phase
    // Prepare RemoveRule Object to be sent
    operation->m_operation_id = 300;
    return_value = NULL;
        //::write (user_address, operation, sizeof (struct ControlOperation));
    if (return_value <= 0) {
        return PStatus::Error ();
    }

    // Receive Phase
    // Create the ControlResponse object to receive (as the response will be an ACK, we may only
    // receive this one)
    // FIXME: this will be an ACK response.
    return_value = NULL;
        //::read (user_address, &response, sizeof (struct ACK));
    if (return_value <= 0) {
        return PStatus::Error ();
    }

    // Process Phase
    // This will be an ACK of the RemoveRule submission ... (possibly remove)
    return_value = NULL;
        //::read (user_address, &response, sizeof (struct ACK));
    if (return_value <= 0) {
        return PStatus::Error ();
    } else {
        status = PStatus::OK ();
        // Logging::DEBUG("Process Phase::REMOVE_RULE::" + std::to_string(response.response));
        fprintf (stdout, "Process Phase::REMOVE_RULE::%d\n", response.m_message);
    }

    return status;
}

// missing actual implementation (this is a placeholder) collect_statistics call. (...)
PStatus LocalInterface::collect_statistics (const std::string& user_address, ControlOperation* operation)
{
    Logging::log_error ("LocalInterface::Get Stats not implemented.");
    return PStatus::NotSupported ();
}

//    CollectStatisticsTF call. (...)
PStatus LocalInterface::collect_global_statistics (const std::string& user_address,
    ControlOperation* operation,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& stats_tf_objects)
{
    controllers_grpc_interface::ControlOperation operation1;

    controllers_grpc_interface::StatsGlobalMap reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    Status status = stub_->CollectGlobalStatistics(&context, operation1, &reply);

    if (!status.ok()) {
        Logging::log_error ("collect_tensorflow_statistics: Error while writing control operation (" + status.error_message() + ").");
        return PStatus::Error();
    } else {
        for (auto stats_tf : reply.stats()){

            stats_tf_objects->emplace(stats_tf.stage_name() + "+" + stats_tf.stage_env(),
                std::make_unique<StageResponseStatsGlobal> (COLLECT_GLOBAL_STATS,
                    stats_tf.m_read_rate(),
                    stats_tf.m_write_rate(),
                    stats_tf.m_open_rate(),
                    stats_tf.m_close_rate(),
                    stats_tf.m_getattr_rate(),
                    stats_tf.m_metadata_total_rate()
                ));
        }


        return PStatus::OK ();
    }

    // verify is logging is enabled to prevent creating a new string unnecessarily
    // if (Logging::is_debug_enabled ()) {
    //     std::stringstream stream;
    //     stream << "StatsTensorFlowRaw: ";
    //     stream << stats_tf_object.m_read_rate / 1024 / 1024 << " - ";
    //     stream << stats_tf_object.m_write_rate / 1024 / 1024 << "\n";
    //     Logging::log_info (stream.str ());
    //

}

//    CollectStatisticsTF call. (...)
PStatus LocalInterface::collect_entity_statistics (const std::string& user_address,
    ControlOperation* operation,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& stats_tf_objects)
{
    // pre-send phase
    // prepare ControlSend object
    /*operation->m_operation_id = -1;
    operation->m_operation_type = COLLECT_GLOBAL_STATS;
    operation->m_operation_subtype = TENSORFLOW_STATISTIC_COLLECTION;
    operation->m_size = sizeof (struct StatsTFControlApplication);
*/
    controllers_grpc_interface::ControlOperation operation1;

    controllers_grpc_interface::StatsEntityMap reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;


    Status status = stub_->CollectEntityStatistics(&context, operation1, &reply);

    if (!status.ok()) {
        Logging::log_error ("collect_tensorflow_statistics: Error while writing control operation (" + status.error_message() + ").");
        return PStatus::Error();
    } else {

        for (auto stats : reply.stats()){
            std::unique_ptr<std::unordered_map<std::string, double>> stats_entities_object =
                std::make_unique<std::unordered_map<std::string, double>>();;

            for(auto stats_entity: stats.second.ent_stats()){
                stats_entities_object->emplace(stats_entity.first, stats_entity.second);
            }
            stats_tf_objects->emplace(stats.first,
                std::make_unique<StageResponseStatsEntity> (COLLECT_ENTITY_STATS, stats_entities_object));
        }
        return PStatus::OK ();
    }

    // verify is logging is enabled to prevent creating a new string unnecessarily
    // if (Logging::is_debug_enabled ()) {
    //     std::stringstream stream;
    //     stream << "StatsTensorFlowRaw: ";
    //     stream << stats_tf_object.m_read_rate / 1024 / 1024 << " - ";
    //     stream << stats_tf_object.m_write_rate / 1024 / 1024 << "\n";
    //     Logging::log_info (stream.str ());
    //

}


void LocalInterface::parse_rule (const std::string& rule, std::vector<std::string>* tokens)
{
    size_t start;
    size_t end = 0;

    while ((start = rule.find_first_not_of ('|', end)) != std::string::npos) {
        end = rule.find ('|', start);
        tokens->push_back (rule.substr (start, end - start));
    }
}


void LocalInterface::fill_housekeeping_rules_grpc(controllers_grpc_interface::LocalSimplifiedHandshakeRaw* housekeeping_rules,
                                                  const std::string& rule)
{

    size_t start;
    size_t end = 0;


    bool first = true;


    while ((start = rule.find_first_not_of (':', end)) != std::string::npos) {
        end = rule.find (':', start);

        // Exclude LOCAL_HANDSHAKE |
        if (first) { first = false; continue; }

        std::string token_rule = rule.substr (start, end - start);
        housekeeping_rules->add_rules (token_rule);
    }
}


void LocalInterface::fill_create_channel_rule_grpc(controllers_grpc_interface::HousekeepingCreateChannelRaw* create_channel,
                                               const std::vector<std::string>& tokens)
{
    create_channel->set_m_rule_id(std::stol (tokens[1]));
    create_channel->set_m_rule_type(static_cast<int> (HousekeepingOperation::create_channel));
    create_channel->set_m_channel_id(std::stol (tokens[3]));
    create_channel->set_m_context_definition(RulesFileParser::convert_context_type_definition (tokens[4]));
    create_channel->set_m_workflow_id(std::stol (tokens[5]));
    create_channel->set_m_operation_type(RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[6]));
    create_channel->set_m_operation_context(RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[7]));
}

void LocalInterface::fill_create_channel_rule (HousekeepingCreateChannelRaw* create_channel,
    const std::vector<std::string>& tokens)
{
    create_channel->m_rule_id = std::stol (tokens[1]);
    create_channel->m_rule_type = static_cast<int> (HousekeepingOperation::create_channel);
    create_channel->m_channel_id = std::stol (tokens[3]);
    create_channel->m_context_definition = RulesFileParser::convert_context_type_definition (tokens[4]);
    create_channel->m_workflow_id = std::stol (tokens[5]);
    create_channel->m_operation_type = RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[6]);
    create_channel->m_operation_context = RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[7]);
}

void LocalInterface::fill_create_object_rule_grpc (controllers_grpc_interface::HousekeepingCreateObjectRaw* create_object,
                                              const std::vector<std::string>& tokens)
{
    create_object->set_m_rule_id(std::stol (tokens[1]));
    create_object->set_m_rule_type(static_cast<int> (HousekeepingOperation::create_object));
    create_object->set_m_channel_id(std::stol (tokens[3]));
    create_object->set_m_enforcement_object_id(std::stol (tokens[4]));
    create_object->set_m_context_definition(RulesFileParser::convert_context_type_definition (tokens[5]));
    create_object->set_m_operation_type(RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[6]));
    create_object->set_m_operation_context(RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[7]));
    create_object->set_m_enforcement_object_type(static_cast<long> (RulesFileParser::convert_object_type (tokens[8])));
    create_object->set_m_property_first(std::stol (tokens[9]));
    create_object->set_m_property_second(std::stol (tokens[10]));
}

void LocalInterface::fill_create_object_rule (HousekeepingCreateObjectRaw* create_object,
    const std::vector<std::string>& tokens)
{
    create_object->m_rule_id = std::stol (tokens[1]);
    create_object->m_rule_type = static_cast<int> (HousekeepingOperation::create_object);
    create_object->m_channel_id = std::stol (tokens[3]);
    create_object->m_enforcement_object_id = std::stol (tokens[4]);
    create_object->m_context_definition = RulesFileParser::convert_context_type_definition (tokens[5]);
    create_object->m_operation_type = RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[6]);
    create_object->m_operation_context = RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[7]);
    create_object->m_enforcement_object_type = static_cast<long> (RulesFileParser::convert_object_type (tokens[8]));
    create_object->m_property_first = std::stol (tokens[9]);
    create_object->m_property_second = std::stol (tokens[10]);
}

void LocalInterface::fill_enforcement_rule_grpc (controllers_grpc_interface::EnforcementRuleRaw* enf_object,
                                            const std::vector<std::string>& tokens)
{
    // convert string to enforcement operation type
    int operation_type
        = RulesFileParser::convert_enforcement_operation (RulesFileParser::convert_object_type (tokens[4]),
                                                          tokens[5]);

    enf_object->set_m_rule_id(std::stoll (tokens[1]));
    enf_object->set_m_channel_id(std::stol (tokens[2]));
    enf_object->set_m_enforcement_object_id(std::stol (tokens[3]));
    enf_object->set_m_enforcement_operation(operation_type);
    enf_object->set_m_property_first(std::stol (tokens[6]));
    if (operation_type == 1) {
        enf_object->set_m_property_second(std::stol (tokens[7]));
    }
}

void LocalInterface::fill_enforcement_rule (EnforcementRuleRaw* enf_object,
    const std::vector<std::string>& tokens)
{
    // convert string to enforcement operation type
    int operation_type
            = RulesFileParser::convert_enforcement_operation (RulesFileParser::convert_object_type (tokens[4]),
                                                   tokens[5]);

    enf_object->m_rule_id = std::stoll (tokens[1]);
    enf_object->m_channel_id = std::stol (tokens[2]);
    enf_object->m_enforcement_object_id = std::stol (tokens[3]);
    enf_object->m_enforcement_operation = operation_type;
    enf_object->m_property_first = std::stol (tokens[6]);
    if (operation_type == 1) {
        enf_object->m_property_second = std::stol (tokens[7]);
    }
}

} // namespace shepherd
