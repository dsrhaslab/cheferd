/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/networking/interface_definitions.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

// PolicyGenerator default constructor.
PolicyGenerator::PolicyGenerator () = default;

// PolicyGenerator default destructor.
PolicyGenerator::~PolicyGenerator () = default;

// convert_housekeeping_create_channel_string call. Convert HousekeepingCreateChannelRaw object
// into string format.
void PolicyGenerator::convert_housekeeping_create_channel_string (
    const HousekeepingCreateChannelRaw& hsk_raw,
    std::string& hsk_rule)
{
    // clean possible old values of string
    hsk_rule.clear ();

    // pass HousekeepingCreateEChannelRaw to string format
    hsk_rule += std::to_string (CREATE_HSK_RULE) + "|" + // rule type
        std::to_string (hsk_raw.m_rule_id) + "|" + "create_channel" + "|"
        + std::to_string (hsk_raw.m_channel_id) + "|"
        + RulesFileParser::convert_context_type_definition (
            static_cast<ContextType> (hsk_raw.m_context_definition))
        + "|" + std::to_string (hsk_raw.m_workflow_id) + "|"
        + RulesFileParser::convert_differentiation_definitions (
            static_cast<ContextType> (hsk_raw.m_context_definition),
            hsk_raw.m_operation_type)
        + "|"
        + RulesFileParser::convert_differentiation_definitions (
            static_cast<ContextType> (hsk_raw.m_context_definition),
            hsk_raw.m_operation_context)
        + "|";
}

// convert_housekeeping_create_object_string call. Convert HousekeepingCreateObjectRaw object
// into string format.
void PolicyGenerator::convert_housekeeping_create_object_string (
    const HousekeepingCreateObjectRaw& hsk_raw,
    std::string& hsk_rule)
{
    // clean possible old values of string
    hsk_rule.clear ();

    // pass HousekeepingCreateEObjectRaw to string format
    hsk_rule += std::to_string (CREATE_HSK_RULE) + "|" + std::to_string (hsk_raw.m_rule_id) + "|"
        + "create_object" + "|" + std::to_string (hsk_raw.m_channel_id) + "|"
        + std::to_string (hsk_raw.m_enforcement_object_id) + "|"
        + RulesFileParser::convert_context_type_definition (
            static_cast<ContextType> (hsk_raw.m_context_definition))
        + "|"
        + RulesFileParser::convert_differentiation_definitions (
            static_cast<ContextType> (hsk_raw.m_context_definition),
            hsk_raw.m_operation_type)
        + "|"
        + RulesFileParser::convert_differentiation_definitions (
            static_cast<ContextType> (hsk_raw.m_context_definition),
            hsk_raw.m_operation_context)
        + "|"
        + RulesFileParser::convert_object_type (
            static_cast<EnforcementObjectType> (hsk_raw.m_enforcement_object_type))
        + "|" + std::to_string (hsk_raw.m_property_first) + "|"
        + std::to_string (hsk_raw.m_property_second) + "|";
}

} // namespace cheferd
