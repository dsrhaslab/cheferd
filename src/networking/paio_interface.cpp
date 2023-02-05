/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/networking/paio_interface.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

// PAIOInterface default constructor.
PAIOInterface::PAIOInterface () = default;

// PAIOInterface default destructor.
PAIOInterface::~PAIOInterface () = default;

// stage_handshake call. Handshake a data plane stage.
// Submit a handshake request to collect data about the data plane stage.
PStatus PAIOInterface::stage_handshake (int socket,
    ControlOperation* operation,
    StageSimplifiedHandshakeRaw& stage_info_obj)
{
    // pre-send phase
    // prepare ControlSend object
    operation->m_operation_id = -1;
    operation->m_operation_type = STAGE_HANDSHAKE;
    operation->m_size = sizeof (struct StageSimplifiedHandshakeRaw);

    // write ControlSend structure through socket
    ssize_t return_value = ::write (socket, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error (
            "PAIOInterface: stage_handshake: Error while writing control operation ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // read StageSimplifiedHandshakeRaw structure from socket
    return_value = ::read (socket, &stage_info_obj, sizeof (struct StageSimplifiedHandshakeRaw));

    // verify total bytes read
    if (return_value != sizeof (struct StageSimplifiedHandshakeRaw)) {
        Logging::log_error ("PAIOInterface: stage_handshake: failed to receive handshake object ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    } else {
        // debug message
        std::stringstream stream;
        stream << "Serialize::StageSimplifiedHandshakeRaw\n";
        stream << "   name\t\t: " << stage_info_obj.m_stage_name;
        stream << " (" << sizeof (stage_info_obj.m_stage_name) << ")\n";
        stream << "   env\t\t: " << stage_info_obj.m_stage_env;
        stream << " (" << sizeof (stage_info_obj.m_stage_env) << ")\n";
        stream << "   pid\t\t: " << stage_info_obj.m_pid << "\n";
        stream << "   ppid\t\t: " << stage_info_obj.m_ppid << "\n";
        stream << "   hostname\t\t: " << stage_info_obj.m_stage_hostname;
        stream << " (" << sizeof (stage_info_obj.m_stage_hostname) << ")\n";
        stream << "   user\t\t: " << stage_info_obj.m_stage_user;
        stream << " (" << sizeof (stage_info_obj.m_stage_user) << ")\n";
        stream << "Size of struct: " << sizeof (StageSimplifiedHandshakeRaw) << "\n";
        Logging::log_debug (stream.str ());

        return PStatus::OK ();
    }
}

// stage_handshake_address call. Informs a data plane stage about the new socket to connect to.
PStatus PAIOInterface::stage_handshake_address (int socket, const std::string& rule, ACK& response)
{

    std::vector<std::string> tokens {};
    this->parse_rule (rule, &tokens);

    StageHandshakeRaw handshake_object;

    strcpy (handshake_object.m_address, tokens[1].c_str ());

    handshake_object.m_port = std::stoi (tokens[2]);

    // write ControlSend structure through socket
    ssize_t return_value = ::write (socket, &handshake_object, sizeof (struct StageHandshakeRaw));

    // verify total written bytes
    if (return_value != sizeof (struct StageHandshakeRaw)) {
        Logging::log_error (
            "PAIOInterface: stage_handshake_address: Error while writing address:port ("
            + std::to_string (return_value) + ").");
        response.m_message = static_cast<int> (AckCode::error);
        return PStatus::Error ();
    } else {
        response.m_message = static_cast<int> (AckCode::ok);
        return PStatus::OK ();
    }
}

// create_housekeeping_rule call. Creates a housekeeping rule at the data plane stage.
PStatus PAIOInterface::create_housekeeping_rule (int socket,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    Logging::log_debug ("PAIOInterface: create_housekeeping_rule: " + rule);

    // parsing phase
    std::vector<std::string> rule_tokens {};
    this->parse_rule (rule, &rule_tokens);

    if (rule_tokens.empty ()) {
        Logging::log_error ("PAIOInterface: create_housekeeping_rule: empty rule.");
        return PStatus::Error ();
    }

    for (int i = 0; i < rule_tokens.size (); i++) {
        std::cout << i << " -- " << rule_tokens[i] << "\n";
    }

    // prepare ControlSend object
    ssize_t return_value;
    operation->m_operation_id = 10; // fixme: update the identifier value
    operation->m_operation_type = CREATE_HSK_RULE;
    operation->m_operation_subtype
        = static_cast<int> (RulesFileParser::convert_housekeeping_operation (rule_tokens[2]));

    switch (operation->m_operation_subtype) {
        case HSK_CREATE_CHANNEL: {
            operation->m_size = sizeof (struct HousekeepingCreateChannelRaw);

            // prepare HousekeepingCreateChannelRaw object to be sent
            HousekeepingCreateChannelRaw create_channel_rule {};
            this->fill_create_channel_rule (&create_channel_rule, rule_tokens);

            // pre-send phase
            return_value = ::write (socket, operation, sizeof (struct ControlOperation));
            // verify total written bytes
            if (return_value != sizeof (struct ControlOperation)) {
                Logging::log_error ("PAIOInterface: create_housekeeping_rule (channel): Error "
                                    "while writing control operation ("
                    + std::to_string (return_value) + ").");
                return PStatus::Error ();
            }

            // send phase
            return_value = ::write (socket,
                &create_channel_rule,
                sizeof (struct HousekeepingCreateChannelRaw));

            // verify total written bytes
            if (return_value != sizeof (struct HousekeepingCreateChannelRaw)) {
                Logging::log_error ("PAIOInterface: create_housekeeping_rule (channel): Error "
                                    "while writing housekeeping rule ("
                    + std::to_string (return_value) + ").");
                return PStatus::Error ();
            }

            break;
        }

        case HSK_CREATE_OBJECT: {
            operation->m_size = sizeof (struct HousekeepingCreateObjectRaw);

            // prepare HousekeepingCreateObjectRaw object to be sent
            HousekeepingCreateObjectRaw create_object_rule {};
            this->fill_create_object_rule (&create_object_rule, rule_tokens);

            // pre-send phase
            return_value = ::write (socket, operation, sizeof (struct ControlOperation));

            // verify total written bytes
            if (return_value != sizeof (struct ControlOperation)) {
                Logging::log_error ("PAIOInterface: create_housekeeping_rule (object): Error while "
                                    "writing control operation ("
                    + std::to_string (return_value) + ").");
                return PStatus::Error ();
            }

            // send phase
            return_value = ::write (socket,
                &create_object_rule,
                sizeof (struct HousekeepingCreateObjectRaw));

            // verify total written bytes
            if (return_value != sizeof (struct HousekeepingCreateObjectRaw)) {
                Logging::log_error ("PAIOInterface: create_housekeeping_rule (object): Error while "
                                    "writing housekeeping rule ("
                    + std::to_string (return_value) + ").");
                return PStatus::Error ();
            }

            break;
        }

        default:
            Logging::log_error ("PAIOInterface: After parsing -- other rule");
            return PStatus::Error ();
    }

    // receive phase
    return_value = ::read (socket, &response, sizeof (struct ACK));
    if (return_value <= 0 || response.m_message == static_cast<int> (AckCode::error)) {
        Logging::log_error ("PAIOInterface: create_housekeeping_rule: Error while reading ACK "
                            "message from data plane stage ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    } else if (response.m_message == static_cast<int> (AckCode::ok)) {
        Logging::log_debug ("PAIOInterface: create_housekeeping_rule: ACK message received ("
            + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error ();
    }
}

// mark_stage_ready call. Mark data plane stage as ready.
PStatus PAIOInterface::mark_stage_ready (int socket,
    ControlOperation* operation,
    StageReadyRaw& stage_ready_obj,
    ACK& response)
{
    // pre-send phase
    // prepare ControlSend object
    operation->m_operation_id = -1;
    operation->m_operation_type = STAGE_READY;
    operation->m_size = sizeof (struct StageReadyRaw);

    // write ControlOperation structure through socket
    ssize_t return_value = ::write (socket, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error (
            "PAIOInterface: mark_stage_ready: Error while writing control operation ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // send phase
    stage_ready_obj.m_mark_stage = true;
    return_value = ::write (socket, &stage_ready_obj, sizeof (struct StageReadyRaw));
    // verify total written bytes
    if (return_value != sizeof (struct StageReadyRaw)) {
        Logging::log_error (
            "PAIOInterface: mark_stage_ready (channel): Error while writing stage ready ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // receive phase
    return_value = ::read (socket, &response, sizeof (struct ACK));
    if (return_value <= 0 || response.m_message == static_cast<int> (AckCode::error)) {
        Logging::log_error ("PAIOInterface: mark_stage_ready: Error while reading ACK message from "
                            "data plane stage ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    } else if (response.m_message == static_cast<int> (AckCode::ok)) {
        Logging::log_debug ("PAIOInterface: mark_stage_ready: ACK message received ("
            + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error ();
    }
}

// create_enforcement_rule call. Creates an enforcement rule at the data plane stage.
PStatus PAIOInterface::create_enforcement_rule (int socket,
    ControlOperation* operation,
    const std::string& rule,
    ACK& response)
{
    // validate if logging is enabled and log debug message
    if (Logging::is_debug_enabled ()) {
        Logging::log_debug ("PAIOInterface: create_enforcement_rule: " + rule);
    }

    // parsing phase
    std::vector<std::string> rule_tokens {};
    this->parse_rule (rule, &rule_tokens);

    if (rule_tokens.empty ()) {
        Logging::log_error ("PAIOInterface: create_enforcement_rule: empty rule.");
        return PStatus::Error ();
    }

    // prepare ControlOperation object
    operation->m_operation_id = 10; // fixme: update the identifier value
    operation->m_operation_type = CREATE_ENF_RULE;
    operation->m_size = sizeof (struct EnforcementRuleRaw);

    // prepare EnforcementRuleRaw object to be sent
    EnforcementRuleRaw create_enforcement_rule {};
    this->fill_enforcement_rule (&create_enforcement_rule, rule_tokens);

    // pre-send phase
    ssize_t return_value = ::write (socket, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error (
            "PAIOInterface: create_enforcement_rule: Error while writing control operation ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // send phase
    return_value = ::write (socket, &create_enforcement_rule, sizeof (struct EnforcementRuleRaw));

    // verify total written bytes
    if (return_value != sizeof (struct EnforcementRuleRaw)) {
        Logging::log_error (
            "PAIOInterface: create_enforcement_rule: Error while writing enforcement rule object "
            "to the data plane stage ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // receive phase
    return_value = ::read (socket, &response, sizeof (struct ACK));
    if (return_value <= 0 || response.m_message == static_cast<int> (AckCode::error)) {
        Logging::log_error ("PAIOInterface: create_enforcement_rule: Error while reading ACK "
                            "message from data plane stage ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    } else if (response.m_message == static_cast<int> (AckCode::ok)) {
        Logging::log_debug ("PAIOInterface: create_enforcement_rule: ACK message received ("
            + std::to_string (response.m_message) + ").");
        return PStatus::OK ();
    } else {
        return PStatus::Error ();
    }
}

// RemoveRule call. Remove a HousekeepingRule from a specific data plane stage.
PStatus
PAIOInterface::RemoveRule (int socket, ControlOperation* operation, int rule_id, ACK& response)
{
    PStatus status = PStatus::Error ();
    // Pre-send Phase
    // Prepare ControlOperation object
    operation->m_operation_id = -1;
    operation->m_size = sizeof (struct ControlOperation);

    ssize_t return_value = ::write (socket, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error ("PAIOInterface: remove_rule: Error while writing control operation ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // Send Phase
    // Prepare RemoveRule Object to be sent
    operation->m_operation_id = 300;
    return_value = ::write (socket, operation, sizeof (struct ControlOperation));
    if (return_value <= 0) {
        return PStatus::Error ();
    }

    // Receive Phase
    // Create the ControlResponse object to receive (as the response will be an ACK, we may only
    // receive this one)
    return_value = ::read (socket, &response, sizeof (struct ACK));
    if (return_value <= 0) {
        return PStatus::Error ();
    }

    // Process Phase
    // This will be an ACK of the RemoveRule submission ... (possibly remove)
    return_value = ::read (socket, &response, sizeof (struct ACK));
    if (return_value <= 0) {
        return PStatus::Error ();
    } else {
        status = PStatus::OK ();
        // Logging::DEBUG("Process Phase::REMOVE_RULE::" + std::to_string(response.response));
        fprintf (stdout, "PAIOInterface: Process Phase::REMOVE_RULE::%d\n", response.m_message);
    }

    return status;
}

// collect_statistics csll. Get the statistics of a current data plane stage.
PStatus PAIOInterface::collect_statistics (int socket, ControlOperation* operation)
{
    Logging::log_error ("PAIOInterface: Get Stats not implemented.");
    return PStatus::NotSupported ();
}

// collect_statistics call. Get the statistics of a current data plane stage.
PStatus PAIOInterface::collect_global_statistics (int socket,
    ControlOperation* operation,
    StatsGlobalRaw& stats_tf_object)
{
    // pre-send phase
    // prepare ControlSend object
    operation->m_operation_id = -1;
    operation->m_operation_type = COLLECT_DETAILED_STATS;
    operation->m_operation_subtype = COLLECT_GLOBAL_STATS;
    // operation->m_operation_subtype = TENSORFLOW_STATISTIC_COLLECTION; // this is highly hardcoded
    operation->m_size = sizeof (struct StatsGlobalRaw);

    signal (SIGPIPE, SIG_IGN);

    // write ControlSend structure through socket
    ssize_t return_value = ::write (socket, operation, sizeof (struct ControlOperation));

    // verify total written bytes
    if (return_value != sizeof (struct ControlOperation)) {
        Logging::log_error (
            "PAIOInterface: collect_global_statistics: Error while writing control operation ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    }

    // Read phase
    // read StatsTFRaw structure from socket
    return_value = ::read (socket, &stats_tf_object, sizeof (struct StatsGlobalRaw));

    // verify total bytes read
    if (return_value != sizeof (struct StatsGlobalRaw)) {
        Logging::log_error (
            "PAIOInterface: collect_global_statistics: Error while reading StatsGlobalRaw object "
            "from data plane stage ("
            + std::to_string (return_value) + ").");
        return PStatus::Error ();
    } else {
        // verify is logging is enabled to prevent creating a new string unnecessarily
        // if (Logging::is_debug_enabled ()) {
        std::stringstream stream;
        stream << "StatsGlobal :: ";
        stream << stats_tf_object.m_total_rate / 1024 << "\n";
        Logging::log_info (stream.str ());

        return PStatus::OK ();
    }
}

////////////////////////////////////////////
//////////// Auxiliary Functions ///////////
////////////////////////////////////////////

// parse_rule call: Parses a rule into tokens.
void PAIOInterface::parse_rule (const std::string& rule, std::vector<std::string>* tokens)
{
    size_t start;
    size_t end = 0;

    while ((start = rule.find_first_not_of ('|', end)) != std::string::npos) {
        end = rule.find ('|', start);
        tokens->push_back (rule.substr (start, end - start));
    }
}

// fill_create_channel_rule call. Fill HousekeepingCreateChannelRaw with tokens data.
void PAIOInterface::fill_create_channel_rule (HousekeepingCreateChannelRaw* create_channel,
    const std::vector<std::string>& tokens)
{
    create_channel->m_rule_id = std::stol (tokens[1]);
    create_channel->m_rule_type = static_cast<int> (HousekeepingOperation::create_channel);
    create_channel->m_channel_id = std::stol (tokens[3]);
    create_channel->m_context_definition
        = RulesFileParser::convert_context_type_definition (tokens[4]);
    create_channel->m_workflow_id = std::stol (tokens[5]);
    create_channel->m_operation_type
        = RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[6]);
    create_channel->m_operation_context
        = RulesFileParser::convert_differentiation_definitions (tokens[4], tokens[7]);
}

// fill_create_object_rule call. Fill HousekeepingCreateObjectRaw with tokens data.
void PAIOInterface::fill_create_object_rule (HousekeepingCreateObjectRaw* create_object,
    const std::vector<std::string>& tokens)
{
    create_object->m_rule_id = std::stol (tokens[1]);
    create_object->m_rule_type = static_cast<int> (HousekeepingOperation::create_object);
    create_object->m_channel_id = std::stol (tokens[3]);
    create_object->m_enforcement_object_id = std::stol (tokens[4]);
    create_object->m_context_definition
        = RulesFileParser::convert_context_type_definition (tokens[5]);
    create_object->m_operation_type
        = RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[6]);
    create_object->m_operation_context
        = RulesFileParser::convert_differentiation_definitions (tokens[5], tokens[7]);
    create_object->m_enforcement_object_type
        = static_cast<long> (RulesFileParser::convert_object_type (tokens[8]));
    create_object->m_property_first = std::stol (tokens[9]);
    create_object->m_property_second = std::stol (tokens[10]);
}

// fill_enforcement_rule call. Fill EnforcementRuleRaw with tokens data.
void PAIOInterface::fill_enforcement_rule (EnforcementRuleRaw* enf_object,
    const std::vector<std::string>& tokens)
{
    // convert string to enforcement operation type
    int operation_type = RulesFileParser::convert_enforcement_operation (
        RulesFileParser::convert_object_type (tokens[4]),
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

} // namespace cheferd
