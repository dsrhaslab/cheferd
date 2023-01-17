/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/networking/interface_definitions.hpp>
#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

//    PolicyGenerator default constructor.
PolicyGenerator::PolicyGenerator () = default;

//    PolicyGenerator default destructor.
PolicyGenerator::~PolicyGenerator () = default;

//    generateHousekeepingRule call. Generate a HousekeepingRule with following
//    core properties.
std::string
PolicyGenerator::generateHousekeepingRule (int rule_type, int rule_id, int enf_unit_id) const
{
    std::string rule_tmp {};

    rule_tmp += std::to_string (CREATE_HSK_RULE) + "|" + // rule type
        std::to_string (rule_id) + "|" + // rule identifier
        std::to_string (rule_type) + "|" + // operation tag
        std::to_string (enf_unit_id) + "|"; // EnforcementUnit identifier

    switch (rule_type) {
        case HSK_CREATE_CHANNEL:
            rule_tmp +=
                //                    EnforcementChannelMode
                std::to_string ((int)EnforcementChannelMode::SYNC) + "|" +
                //                    DifferentiationContext
                std::to_string ((int)DiffContext::FLOW) + "|" +
                //                    OperationContext
                std::to_string ((int)OperationContext::BG_NO_OP);

            break;

        case HSK_CREATE_OBJECT:
            rule_tmp +=
                //                    DifferentiationContext
                std::to_string ((int)DiffContext::FLOW) + "|" +
                //                    OperationContext
                std::to_string ((int)OperationContext::BG_NO_OP) + "|" +
                //                    EnforcementObject type (DynamicRateLimiting)
                std::to_string ((int)EnforcementObjectType::DRL) + "|" +
                //                    Property first: 1s refill period (1 000 000
                //                    microseconds)
                std::to_string (1000000) + "|" +
                //                    Property second: 5K IOPS
                std::to_string (5000) + "|" +
                //                    Property third: 10K IOPS
                std::to_string (10000);

            break;

        default:
            return "";
    }

    return rule_tmp;
}

std::string PolicyGenerator::generateEnforcementRule (int enf_unit_id,
    const DiffContext& differentiation_context,
    const OperationContext& operation_context,
    int enforcement_operation)
{
    std::string enf_rule_t {};

    enf_rule_t += std::to_string (CREATE_ENF_RULE) + "|" + // rule type
        std::to_string (enf_unit_id) + "|" + // EnforcementUnit's identifier
        std::to_string ((int)differentiation_context) + "|" + // DifferentiationContext
        std::to_string ((int)operation_context) + "|" + // OperationContext
        std::to_string (enforcement_operation) + "|"; // EnforcementOperation type

    switch (enforcement_operation) {
        case ENF_INIT: {
            long refill_period_t = 1000000;
            long rate_t = (random () % 10000);
            long max_rate_t = rate_t + (random () % 10000);

            enf_rule_t += std::to_string (refill_period_t) + "|" + // DRL's refill period
                std::to_string (rate_t) + "|" + // DRL's rate
                std::to_string (max_rate_t); // DRL's maximum rate
            break;
        }

        case ENF_RATE: {
            long rate_t = (random () % 10000);
            enf_rule_t += std::to_string (rate_t); // DRL's rate
            break;
        }

        case ENF_MAX_RATE: {
            long max_rate_t = (random () % 20000);
            enf_rule_t += std::to_string (max_rate_t); // DRL's rate
            break;
        }

        case ENF_REF_WINDOW: {
            long refill_window_t = (random () % 10000000);
            enf_rule_t += std::to_string (refill_window_t); // DRL's refill period
            break;
        }
        case ENF_PREEMPT:
        case ENF_NONE:
        default:
            break;
    }

    return enf_rule_t;
}

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

void PolicyGenerator::ConvertDifferentiationRuleRawToString (const DifferentiationRuleRaw& diff_raw,
    std::string& dif_rule)
{ }

void PolicyGenerator::convert_enforcement_rule_string (const EnforcementRuleRaw& enf_raw,
    std::string& enf_rule)
{
    // clean possible old values of string
    enf_rule.clear ();

    std::string operation
        = RulesFileParser::convert_enforcement_operation (enf_raw.m_enforcement_operation);
    std::string object_type = "drl";
    if (operation == "noop") {
        object_type = "noop";
    }

    // pass EnforcementRuleRaw to string format
    enf_rule += std::to_string (CREATE_ENF_RULE) + "|" + std::to_string (enf_raw.m_rule_id) + "|"
        + std::to_string (enf_raw.m_channel_id) + "|"
        + std::to_string (enf_raw.m_enforcement_object_id) + "|" + object_type + "|" + operation
        + "|" + std::to_string (enf_raw.m_property_first) + "|"
        + std::to_string (enf_raw.m_property_second) + "|"
        + std::to_string (enf_raw.m_property_third) + "|";
}

void PolicyGenerator::convert_enforcement_rule_with_time_string (
    const EnforcementRuleWithTimeRaw& enf_raw,
    std::string& enf_rule)
{
    // clean possible old values of string
    enf_rule.clear ();

    std::string operation
        = RulesFileParser::convert_enforcement_operation (enf_raw.m_enforcement_operation);
    std::string object_type = "drl";
    if (operation == "noop") {
        object_type = "noop";
    }

    // pass EnforcementRuleRaw to string format
    enf_rule += std::to_string (CREATE_ENF_RULE) + "|" + std::to_string (enf_raw.m_rule_id) + "|"
        + std::to_string (enf_raw.m_channel_id) + "|"
        + std::to_string (enf_raw.m_enforcement_object_id) + "|" + operation + "|"
        + std::to_string (enf_raw.m_property_first) + "|"
        + std::to_string (enf_raw.m_property_second) + "|"
        + std::to_string (enf_raw.m_property_third) + "|";
}

} // namespace cheferd
