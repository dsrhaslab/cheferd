/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_POLICY_GENERATOR_HPP
#define SHEPHERD_POLICY_GENERATOR_HPP

#include <shepherd/utils/options.hpp>
#include <string>

namespace shepherd {

class PolicyGenerator {

public:
    /**
     * PolicyGenerator default constructor.
     */
    PolicyGenerator ();

    /**
     * PolicyGenerator default destructor.
     */
    ~PolicyGenerator ();

    /**
     * generateHousekeepingRule:
     * @param rule_type
     * @param rule_id
     * @return
     */
    std::string generateHousekeepingRule (int rule_type, int rule_id, int enf_unit_id) const;

    /**
     * generateDifferentiationRule:
     * @param rule_id
     * @return
     */
    std::string generateDifferentiationRule (int rule_id);

    /**
     * generateEnforcementRule:
     * @return
     */
    std::string generateEnforcementRule (int enf_unit_id,
        const DiffContext& differentiation_context,
        const OperationContext& operation_context,
        int enforcement_operation);


    void convert_housekeeping_create_channel_string (const HousekeepingCreateChannelRaw& hsk_raw,
        std::string& hsk_rule);

    void convert_housekeeping_create_object_string (const HousekeepingCreateObjectRaw& hsk_raw,
        std::string& hsk_rule);

    void ConvertDifferentiationRuleRawToString (const DifferentiationRuleRaw& diff_raw,
        std::string& hsk_rule);

    void convert_enforcement_rule_string (const EnforcementRuleRaw& enf_raw,
        std::string& hsk_rule);

    void convert_enforcement_rule_with_time_string (const EnforcementRuleWithTimeRaw& enf_raw,
        std::string& enf_rule);
};

} // namespace shepherd

#endif // SHEPHERD_POLICY_GENERATOR_HPP
