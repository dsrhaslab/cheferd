/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/utils/rules_file_parser.hpp>

namespace shepherd {

void TestReadFile (RulesFileParser* parser, const std::string& path)
{
    parser->readRulesFromFile (path);
}

void TestPrintStagedRules (RulesFileParser* parser)
{
    parser->printStagedRules ();
}

void TestGetHousekeepingRulesCreateUnit (RulesFileParser* parser, int total_rules)
{
    Logging::log_info (":: GetHousekeepingRulesCreateUnit ::");

    std::vector<HousekeepingCreateEUnitRaw> hsk_rules {};
    parser->getHousekeepingRulesCreateUnit (hsk_rules, total_rules);

    for (auto raw_rule : hsk_rules) {
        Logging::log_info ("Rule " + std::to_string (raw_rule.rule_id) + ": "
            + std::to_string (raw_rule.operation_tag) + ", "
            + std::to_string (raw_rule.enforcement_unit_id) + ", " + std::to_string (raw_rule.mode)
            + ".");
    }
}

void TestGetHousekeepingRulesCreateChannel (RulesFileParser* parser, int total_rules)
{
    Logging::log_info (":: GetHousekeepingRulesCreateChannel ::");

    std::vector<HousekeepingCreateEChannelRaw> hsk_rules {};
    parser->getHousekeepingRulesCreateChannel (hsk_rules, total_rules);

    for (auto raw_rule : hsk_rules) {
        Logging::log_info ("Rule " + std::to_string (raw_rule.rule_id) + ": "
            + std::to_string (raw_rule.operation_tag) + ", "
            + std::to_string (raw_rule.enforcement_unit_id) + ", "
            + std::to_string (raw_rule.channel_mode) + ", "
            + std::to_string (raw_rule.differentiation_context) + ", "
            + std::to_string (raw_rule.operation_context) + ".");
    }
}

void TestGetHousekeepingRulesCreateObject (RulesFileParser* parser, int total_rules)
{
    Logging::log_info (":: GetHousekeepingRulesCreateObject ::");

    std::vector<HousekeepingCreateEObjectRaw> hsk_rules {};
    parser->getHousekeepingRulesCreateObject (hsk_rules, total_rules);

    for (auto raw_rule : hsk_rules) {
        Logging::log_info ("Rule " + std::to_string (raw_rule.rule_id) + ": "
            + std::to_string (raw_rule.operation_tag) + ", "
            + std::to_string (raw_rule.enforcement_unit_id) + ", "
            + std::to_string (raw_rule.differentiation_context) + ", "
            + std::to_string (raw_rule.operation_context) + ", "
            + std::to_string (raw_rule.object_type) + ", "
            + std::to_string (raw_rule.property_first) + ", "
            + std::to_string (raw_rule.property_second) + ", "
            + std::to_string (raw_rule.property_third) + ".");
    }
}

void TestGetHousekeepingRulesAssign (RulesFileParser* parser, int total_rules)
{
    Logging::log_info (":: GetHousekeepingRulesAssign ::");

    std::vector<HousekeepingAssignRaw> hsk_rules {};
    parser->getHousekeepingRulesAssign (hsk_rules, total_rules);

    for (auto raw_rule : hsk_rules) {
        Logging::log_info ("Rule " + std::to_string (raw_rule.rule_id) + ": "
            + std::to_string (raw_rule.operation_tag) + ", "
            + std::to_string (raw_rule.enforcement_unit_id) + ", "
            + std::to_string (raw_rule.flow_id) + ".");
    }
}

void TestGetEnforcementRules (RulesFileParser* parser, int total_rules)
{
    Logging::log_info (":: GetEnforcementRules ::");

    std::vector<EnforcementRuleRaw> enf_rules_t {};
    parser->getEnforcementRules (enf_rules_t, total_rules);

    for (auto raw_rule : enf_rules_t) {
        Logging::log_info ("Rule: " + std::to_string (raw_rule.enforcement_unit_id) + ", "
            + std::to_string (raw_rule.differentiation_context) + ", "
            + std::to_string (raw_rule.operation_context) + ", "
            + std::to_string (raw_rule.enforcement_operation) + ", "
            + std::to_string (raw_rule.property_first) + ", "
            + std::to_string (raw_rule.property_second) + ", "
            + std::to_string (raw_rule.property_third) + ".");
    }
}

void TestEraseRules (RulesFileParser* parser)
{
    Logging::log_info (":: EraseRules ::");
    Logging::log_info ("Number of rules: " + std::to_string (parser->eraseRules ()));
}

} // namespace shepherd

int main ()
{
    // HousekeepingRules parser
    shepherd::RulesFileParser parser_t0 {};
    shepherd::TestReadFile (&parser_t0,
        "/home/rgmacedo/Projects/sds-data-plane/shepherd/files/"
        "default_housekeeping_rules_file");

    shepherd::TestGetHousekeepingRulesCreateUnit (&parser_t0, -1);
    shepherd::TestGetHousekeepingRulesCreateChannel (&parser_t0, -1);
    shepherd::TestGetHousekeepingRulesCreateObject (&parser_t0, -1);
    shepherd::TestGetHousekeepingRulesAssign (&parser_t0, -1);

    shepherd::TestEraseRules (&parser_t0);
    shepherd::TestPrintStagedRules (&parser_t0);

    // EnforcementRules parser
    shepherd::RulesFileParser parser_t1 { shepherd::RuleType::ENF,
        "/home/rgmacedo/Projects/sds-data-plane/shepherd/files/"
        "default_enforcement_rules_file" };

    shepherd::TestGetEnforcementRules (&parser_t1, -1);

    return 0;
}
