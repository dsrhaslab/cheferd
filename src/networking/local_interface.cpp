/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/networking/local_interface.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

/**
 */

namespace cheferd {

// LocalInterface default constructor.
// LocalInterface::LocalInterface () = default;

LocalInterface::LocalInterface (const std::string& user_address) :
    stub_ (GlobalToLocal::NewStub (
        grpc::CreateChannel (user_address, grpc::InsecureChannelCredentials ())))
{ }

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

    operation1.set_m_operation_id (-1);
    operation1.set_m_operation_type (LOCAL_HANDSHAKE);
    operation1.set_m_size (sizeof (struct StageSimplifiedHandshakeRaw));

    // parsing phase
    controllers_grpc_interface::LocalSimplifiedHandshakeRaw housekeeping_rules;
    fill_housekeeping_rules_grpc (&housekeeping_rules, rule);

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->LocalHandshake (&context, housekeeping_rules, &reply);

    if (!status.ok ()) {
        Logging::log_error ("stage_handshake: Error while handshake control operation ("
            + status.error_message () + ").");
        return PStatus::Error ();
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

    operation1.set_m_operation_id (-1);
    operation1.set_m_operation_type (STAGE_HANDSHAKE);
    operation1.set_m_size (sizeof (struct StageSimplifiedHandshakeRaw));

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StageHandshake (&context, operation1, &reply);

    if (!status.ok ()) {
        Logging::log_error ("stage_handshake: Error while handshake control operation ("
            + status.error_message () + ").");
        return PStatus::Error ();
    } else {
        std::cout << "stage_handshake: control operation submitted\n";

        std::cout << "LocalInterface: ControlOperation: ";
        std::cout << operation->m_operation_id << ", ";
        std::cout << operation->m_operation_type << ", ";
        std::cout << operation->m_operation_subtype << ", ";
        std::cout << operation->m_size << "\n";

        strcpy (stage_info_obj.m_stage_name, reply.m_stage_name ().c_str ());
        strcpy (stage_info_obj.m_stage_env, reply.m_stage_env ().c_str ());
        stage_info_obj.m_pid = reply.m_pid ();
        stage_info_obj.m_ppid = reply.m_ppid ();

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
        Logging::log_debug (stream.str ());

        return PStatus::OK ();
    }
}

PStatus LocalInterface::mark_stage_ready (const std::string& user_address,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{

    controllers_grpc_interface::ACK reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // parsing phase
    std::vector<std::string> rule_tokens {};
    this->parse_rule (rule, &rule_tokens, '|');

    controllers_grpc_interface::StageReadyRaw stage_ready_raw;
    stage_ready_raw.set_m_mark_stage (true);
    stage_ready_raw.set_stage_name_env (rule_tokens[1]);

    Status status = stub_->MarkStageReady (&context, stage_ready_raw, &reply);

    response.m_message = reply.m_message ();

    if (!status.ok ()) {
        Logging::log_error ("mark_stage_ready (channel): Error while writing stage ready ("
            + status.error_message () + ").");
        return PStatus::Error ();
    } else if (reply.m_message () == static_cast<int> (AckCode::ok)) {
        Logging::log_debug ("mark_stage_ready: ACK message received ("
            + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error ();
    }
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

    size_t start0;
    size_t end0 = 0;
    bool first = true;

    controllers_grpc_interface::EnforcementRules create_enforcement_rule;
    auto& op_map = *create_enforcement_rule.mutable_operation_rules ();

    while ((start0 = rule.find_first_not_of ('.', end0)) != std::string::npos) {
        end0 = rule.find ('.', start0);

        // Exclude LOCAL_HANDSHAKE |
        if (first) {
            first = false;
            continue;
        }

        std::string cur_rule = rule.substr (start0, end0 - start0);

        std::vector<std::string> rule_tokens {};
        this->parse_rule (cur_rule, &rule_tokens, '|');

        controllers_grpc_interface::EnforcementOpRules create_enforcement_op_rule;

        create_enforcement_op_rule.set_m_rule_id (std::stoll (rule_tokens[0]));
        create_enforcement_op_rule.set_m_stage_name (rule_tokens[1]);

        auto& rules_map = *create_enforcement_op_rule.mutable_env_rates ();

        size_t start1;
        size_t end1 = 0;

        while ((start1 = rule_tokens[3].find_first_not_of ('*', end1)) != std::string::npos) {
            end1 = rule_tokens[3].find ('*', start1);

            std::string token_rule = rule_tokens[3].substr (start1, end1 - start1);

            size_t start2;
            size_t end2 = 0;

            std::vector<std::string> tokens = {};
            while ((start2 = token_rule.find_first_not_of (':', end2)) != std::string::npos) {
                end2 = token_rule.find (':', start2);
                tokens.push_back (token_rule.substr (start2, end2 - start2));
            }

            auto env = std::stoll (tokens[0]);
            rules_map[env] = std::stoll (tokens[1]);
        }

        op_map[rule_tokens[2]] = create_enforcement_op_rule;
    }

    controllers_grpc_interface::ACK reply;
    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // write EnforcementRule object through user_address
    Status status = stub_->CreateEnforcementRule (&context, create_enforcement_rule, &reply);

    response.m_message = reply.m_message ();
    if (!status.ok ()) {
        Logging::log_error ("create_enforcement_rule: Error while writing enforcement rule object "
                            "to the local controler ("
            + status.error_message () + ").");
        return PStatus::Error ();
    } else if (reply.m_message () == static_cast<int> (AckCode::ok)) {
        Logging::log_debug ("create_enforcement_rule: ACK message received ("
            + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error ();
    }
}

//    CollectStatisticsTF call. (...)
PStatus LocalInterface::collect_global_statistics (const std::string& user_address,
    ControlOperation* operation,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
        stats_tf_objects)
{
    controllers_grpc_interface::ControlOperation operation1;

    controllers_grpc_interface::StatsGlobalMap reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    Status status = stub_->CollectGlobalStatistics (&context, operation1, &reply);

    if (!status.ok ()) {
        Logging::log_error ("collect_tensorflow_statistics: Error while writing control operation ("
            + status.error_message () + ").");
        return PStatus::Error ();
    } else {
        for (auto stats : reply.gl_stats ()) {

            stats_tf_objects->emplace (stats.first,
                std::make_unique<StageResponseStat> (COLLECT_GLOBAL_STATS,
                    stats.second.m_metadata_total_rate ()));
        }

        return PStatus::OK ();
    }
}

//    CollectStatisticsTF call. (...)
PStatus LocalInterface::collect_global_statistics_aggregated (const std::string& user_address,
    ControlOperation* operation,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
        stats_tf_objects)
{
    controllers_grpc_interface::ControlOperation operation1;

    controllers_grpc_interface::StatsGlobalMap reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    Status status = stub_->CollectGlobalStatistics (&context, operation1, &reply);

    if (!status.ok ()) {
        Logging::log_error ("collect_tensorflow_statistics: Error while writing control operation ("
            + status.error_message () + ").");
        return PStatus::Error ();
    } else {
        for (auto stats : reply.gl_stats ()) {

            stats_tf_objects->emplace (stats.first,
                std::make_unique<StageResponseStat> (COLLECT_GLOBAL_STATS,
                    stats.second.m_metadata_total_rate ()));
        }

        return PStatus::OK ();
    }
}

void LocalInterface::parse_rule (const std::string& rule,
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

void LocalInterface::fill_housekeeping_rules_grpc (
    controllers_grpc_interface::LocalSimplifiedHandshakeRaw* housekeeping_rules,
    const std::string& rule)
{

    size_t start;
    size_t end = 0;

    bool first = true;

    while ((start = rule.find_first_not_of (':', end)) != std::string::npos) {
        end = rule.find (':', start);

        // Exclude LOCAL_HANDSHAKE |
        if (first) {
            first = false;
            continue;
        }

        std::string token_rule = rule.substr (start, end - start);
        housekeeping_rules->add_rules (token_rule);
    }
}

} // namespace cheferd
