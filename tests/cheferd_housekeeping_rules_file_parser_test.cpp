/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

int TestCreateChannelRules ()
{

    PolicyGenerator generator {};

    // create parser object. Rules are parsed at creation time.
    RulesFileParser file_parser { RuleType::housekeeping,
        "../files/posix_layer_housekeeping_rules_static_test" };
    int rules_size;

    // create, insert, and execute HousekeepingRules of type HSK_CREATE_CHANNEL
    std::vector<HousekeepingCreateChannelRaw> hsk_create_channel {};
    rules_size = file_parser.get_create_channel_rules (hsk_create_channel, -1);

    if (rules_size != 1) {
        return 1;
    }

    // Check a conversion
    // 1 create_channel 1000 posix 1000 total no_op
    HousekeepingCreateChannelRaw hsk_create_channel_test = hsk_create_channel.at (0);

    if (hsk_create_channel_test.m_rule_type
        != static_cast<int> (HousekeepingOperation::create_channel)) {
        return 1;
    } else if (hsk_create_channel_test.m_channel_id != 1000) {
        return 1;
    } else if (hsk_create_channel_test.m_context_definition
        != static_cast<long> (ContextType::POSIX_META)) {
        return 1;
    } else if (hsk_create_channel_test.m_workflow_id != 1000) {
        return 1;
    } else if (hsk_create_channel_test.m_operation_type != static_cast<long> (POSIX_META::no_op)) {
        return 1;
    } else if (hsk_create_channel_test.m_operation_context
        != static_cast<long> (POSIX_META::meta_op)) {
        return 1;
    }

    // Check a conversion to string
    // 1 create_channel 1000 posix 1000 total no_op
    std::string hsk_enf_channel;
    generator.convert_housekeeping_create_channel_string (hsk_create_channel.at (0),
        hsk_enf_channel);

    if (hsk_enf_channel.compare ("4|1|create_channel|1000|posix_meta|1000|no_op|meta_op|") != 0) {
        return 1;
    }

    for (int i = 0; i < rules_size; i++) {
        generator.convert_housekeeping_create_channel_string (hsk_create_channel.at (i),
            hsk_enf_channel);
        Logging::log_info (hsk_enf_channel);
    }

    return 0;
}

int TestCreateObjectRules ()
{

    PolicyGenerator generator {};

    // create parser object. Rules are parsed at creation time.
    RulesFileParser file_parser { RuleType::housekeeping,
        "../files/posix_layer_housekeeping_rules_static_test" };
    int rules_size;

    std::vector<HousekeepingCreateObjectRaw> hsk_create_object {};
    rules_size = file_parser.get_create_object_rules (hsk_create_object, -1);

    if (rules_size != 1) {
        return 1;
    }

    // Check a conversion
    // 5 create_object 1000 1 posix total no_op drl 10000 1073741824
    HousekeepingCreateObjectRaw hsk_create_object_test = hsk_create_object.at (0);

    if (hsk_create_object_test.m_rule_type
        != static_cast<int> (HousekeepingOperation::create_object)) {
        return 1;
    } else if (hsk_create_object_test.m_channel_id != 1000) {
        return 1;
    } else if (hsk_create_object_test.m_enforcement_object_id != 1) {
        return 1;
    } else if (hsk_create_object_test.m_context_definition
        != static_cast<long> (ContextType::POSIX_META)) {
        return 1;
    } else if (hsk_create_object_test.m_operation_type != static_cast<long> (POSIX_META::no_op)) {
        return 1;
    } else if (hsk_create_object_test.m_operation_context
        != static_cast<long> (POSIX_META::meta_op)) {
        return 1;
    } else if (hsk_create_object_test.m_enforcement_object_type
        != static_cast<long> (EnforcementObjectType::NOOP)) {
        return 1;
    } else if (hsk_create_object_test.m_property_first != 1000) {
        return 1;
    } else if (hsk_create_object_test.m_property_second != 50000) {
        return 1;
    }

    // Check a conversion to string
    // 15 create_object 1000 1 posix total no_op drl 10000 1073741824
    std::string hsk_enf_object;
    generator.convert_housekeeping_create_object_string (hsk_create_object.at (0), hsk_enf_object);
    if (hsk_enf_object.compare (
            "4|2|create_object|1000|1|posix_meta|no_op|meta_op|noop|1000|50000|")
        != 0) {
        return 1;
    }

    for (int i = 0; i < rules_size; i++) {
        generator.convert_housekeeping_create_object_string (hsk_create_object.at (i),
            hsk_enf_object);
        Logging::log_info (hsk_enf_object);
    }

    return 0;
}

} // namespace cheferd

int main ()
{
    int res1 = cheferd::TestCreateChannelRules ();
    int res2 = cheferd::TestCreateObjectRules ();
    return res1 || res2;
}
