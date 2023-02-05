/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_POLICY_GENERATOR_HPP
#define CHEFERD_POLICY_GENERATOR_HPP

#include <cheferd/utils/options.hpp>
#include <string>

namespace cheferd {

/**
 * PolicyGenerator class.
 * PolicyGenerator generates policies.
 */
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
     * convert_housekeeping_create_channel_string: Convert HousekeepingCreateChannelRaw object
     * into string format.
     * @param hsk_raw HousekeepingCreateChannelRaw object to be converted into string format.
     * @param hsk_rule Rule in string format.
     */
    void convert_housekeeping_create_channel_string (const HousekeepingCreateChannelRaw& hsk_raw,
        std::string& hsk_rule);

    /**
     * convert_housekeeping_create_object_string: Convert HousekeepingCreateObjectRaw object
     * into string format.
     * @param hsk_raw HousekeepingCreateObjectRaw object to be converted into string format.
     * @param hsk_rule Rule in string format.
     */
    void convert_housekeeping_create_object_string (const HousekeepingCreateObjectRaw& hsk_raw,
        std::string& hsk_rule);
};

} // namespace cheferd

#endif // CHEFERD_POLICY_GENERATOR_HPP
