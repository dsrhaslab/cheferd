/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_RULES_FILE_PARSER_HPP
#define SHEPHERD_RULES_FILE_PARSER_HPP

#include <fstream>
#include <iostream>
#include <shepherd/networking/interface_definitions.hpp>
#include <shepherd/utils/logging.hpp>
#include <vector>

namespace shepherd {

/**
 * RuleType enum class.
 * Defines the type of rules to be parsed, submitted, received, and handled.
 * Currently, it supports the following type:
 *  - housekeeping: respects to HousekeepingRules, which are used for general data plane management,
 *  including creation and configuration of Channels and EnforcementObjects;
 *  - differentiation: respects to DifferentiationRules, which are used to classify and
 *  differentiate I/O requests;
 *  - enforcement: respects to EnforcementRules, which are used to dynamically adjust the data plane
 *  state elements at execution time.
 */
enum class RuleType { housekeeping = 1, differentiation = 2, enforcement = 3,  noop = 0 };

class RulesFileParser {

private:
    RuleType m_rules_type { RuleType::noop };
    std::vector<std::vector<std::string>> m_staged_rules {};
    const int m_create_channel_rules_min_elements { 7 };
    const int m_create_object_rules_min_elements { 8 };

    /**
     * read_rules_from_file: Read rules from a given file and stored them in the m_staged_rules
     * container.
     * @param path Path to the file that contains the rules.
     * @return Returns the number of rules stored.
 */
    int read_rules_from_file (const std::string& path);

    /**
     * parse_rule: Split line (rule in string) into tokens.
     * @param rule Rule in string format.
     * @param tokens Vector to store the each token of the string-based rule.
     */
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens);


public:
    /**
     * convert_housekeeping_operation: Convert string-based operation into the respective
     * HousekeepingOperation type.
     * @param operation String-based operation.
     * @return Returns the respective HousekeepingOperation; returns no-op for unlisted operations.
     */
    static HousekeepingOperation convert_housekeeping_operation (const std::string& operation);

    /**
     * convert_object_type: Convert string-based object type into the respective
     * EnforcementObjectType classifier.
     * @param object_type String-based enforcement object type.
     * @return Returns the respective EnforcementObjectType; returns ::NOOP for unlisted types.
     */
    static EnforcementObjectType convert_object_type (const std::string& object_type);

    static std::string convert_object_type (const EnforcementObjectType& object_type);

    /**
     * convert_enforcement_operation: Convert string-based enforcement operations into a listed
     * integer, based on the respective EnforcementObjectType.
     * @param object_type EnforcementObjectType of the operation to be executed.
     * @param operation String-based operation type.
     * @return Returns an integer that corresponds to the respective enforcement operation
     * (configuration).
     */
    static int convert_enforcement_operation (const EnforcementObjectType& object_type,
                                       const std::string& operation);

    static std::string convert_enforcement_operation (const int& operation);

    /**
     * convert_context_type_definition: Convert a string-based ContextType object to the
     * corresponding long value. Currently, it supports the following ContextType objects:
     * PAIO_GENERAL, POSIX, LSM_KVS_SIMPLE, LSM_KVS_DETAILED, and KVS.
     * @param context_type String-based ContextType object.
     * @return Returns the corresponding long value of the ContextType; if the object is not
     * recognized, it returns -1.
     */
    static int convert_context_type_definition (const std::string& context_type);

    static std::string convert_context_type_definition (const ContextType& context_type);

    /**
     * convert_differentiation_definitions: Convert I/O classification and differentiation
     * definitions from string-based format to the corresponding long value.
     * @param context_type String-based ContextType object, to select the correct conversion
     * method to use.
     * @param definition String-based definition for the I/O differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_differentiation_definitions (const std::string& context_type,
                                              const std::string& definition);

    static std::string convert_differentiation_definitions (const ContextType& context_type,
                                                     const int& definition);

    /**
     * convert_paio_general_definitions: Convert PAIO_GENERAL differentiation definitions from a
     * string-based format to the corresponding long value.
     * @param general_definitions String-based definition of a PAIO_GENERAL element for the I/O
     * differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_paio_general_definitions (const std::string& general_definitions);


    static std::string convert_paio_general_definitions (const PAIO_GENERAL& general_definitions);

    /**
     * convert_posix_lsm_simple_definitions: Convert LSM_KVS_SIMPLE differentiation
     * definitions from a string-based format to the corresponding long value.
     * @param posix_lsm_definitions String-based definition of a LSM_KVS_SIMPLE element for
     * the I/O differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_posix_lsm_simple_definitions (const std::string& posix_lsm_definitions);

    static std::string convert_posix_lsm_simple_definitions (const LSM_KVS_SIMPLE& posix_lsm_definitions);

    /**
     * convert_posix_lsm_detailed_definitions: Convert LSM_KVS_DETAILED differentiation
     * definitions from a string-based format to the corresponding long value.
     * @param posix_lsm_definitions String-based definition of a LSM_KVS_DETAILED element for
     * the I/O differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_posix_lsm_detailed_definitions (const std::string& posix_lsm_definitions);

    static std::string convert_posix_lsm_detailed_definitions (const LSM_KVS_DETAILED& posix_lsm_definitions);

    /**
     * convert_posix_definitions: Convert POSIX differentiation definitions from a string-based
     * format to the corresponding long value.
     * @param posix_definitions String-based definition of a POSIX element for the I/O
     * differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_posix_definitions (const std::string& posix_definitions);

    static std::string convert_posix_definitions (const POSIX& posix_definitions);

    /**
     * convert_posix_meta_definitions: Convert POSIX_META differentiation definitions from a
     * string-based format to the corresponding long value.
     * @param posix_meta_definitions String-based definition of a POSIX_META element for the I/O
     * differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_posix_meta_definitions (const std::string& posix_meta_definitions);

    static std::string convert_posix_meta_definitions (const POSIX_META& posix_meta_definitions);

    /**
     * convert_kvs_definitions: Convert KVS differentiation definitions from a string-based format
     * to the corresponding long value.
     * @param kvs_definitions String-based definition of a KVS element for the I/O differentiation.
     * @return Returns the corresponding long value of the I/O definition.
     */
    static long convert_kvs_definitions (const std::string& kvs_definitions);

    static std::string convert_kvs_definitions (const KVS& kvs_definitions);

    /**
     * RulesFileParser default constructor.
     */
    RulesFileParser ();

    /**
     * RulesFileParser parameterized constructor.
     * @param type type of rules the file comprises. Rules can be of type: HSK
     * (Housekeeping), DIF (Differentiation), and ENF (Enforcement).
     * @param path Path to the file that contains the rules.
     */
    RulesFileParser (RuleType type, const std::string& path);

    /**
     * RulesFileParser default destructor.
     */
    ~RulesFileParser ();

    /**
     * getRulesType: get the type of the rules in the file.
     */
    RuleType get_rule_type () const;

    /**
     * get_create_channel_rules: read raw housekeeping rules of type
     * HSK_CREATE_CHANNEL from vector and store them in hsk_rules.
     * @param hsk_rules reference to a container that stores the RAW structure.
     * @param total_rules number of rules to store in the container (-1
     * indicates to pass all rules).
     * @return returns the number of rules stored in the container.
     */
    int get_create_channel_rules (std::vector<HousekeepingCreateChannelRaw>& hsk_rules,
        int total_rules);

    /**
     * get_create_object_rules: read raw housekeeping rules of type
     * HSK_CREATE_OBJECT from vector and store them in hsk_rules.
     * @param hsk_rules reference to a container that stores the RAW structure.
     * @param total_rules number of rules to store in the container (-1
     * indicates to pass all rules).
     * @return returns the number of rules stored in the container.
     */
    int get_create_object_rules (std::vector<HousekeepingCreateObjectRaw>& hsk_rules,
        int total_rules);

    /**
     * get_enforcement_rules: read raw enforcement rules from staged_rules_ and
     * store them in enf_rules.
     * @param enf_rules Reference to a container that stores the RAW enforcement
     * rules structure.
     * @param total_rules Number of rules to store in the container (-1 indicate
     * to pass all rules).
     * @return Returns the number of rules stored in the container.
     */
    int get_enforcement_rules (std::vector<EnforcementRuleRaw>& enf_rules, int total_rules);

    /**
     * erase_rules: Remove all rules stored in the staged_rules_ container.
     * @return Returns the number of rules in the container.
     */
    int erase_rules ();

    /**
     * print_rules: Write to stdout the rules stored in the staged_rules_
     * container.
     */
    void print_rules () const;
};
} // namespace shepherd

#endif // SHEPHERD_RULES_FILE_PARSER_HPP
