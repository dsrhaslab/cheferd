/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_HOUSEKEEPING_RULE_HPP
#define SHEPHERD_HOUSEKEEPING_RULE_HPP

#include "instance_rule.hpp"
#include "rule.hpp"

namespace shepherd {

class HSKRule : public InstanceRule {
private:
    int rule_type_;
    long rule_id_;
    int housekeeping_operation_;
    long enforcement_unit_id_;
    int enforcement_unit_type_;
    long enforcement_property_;

public:
    HSKRule ();
    HSKRule (long rule, int operation, long unit_id, int unit_type, long property);
    HSKRule (const HSKRule& rule);
    ~HSKRule ();

    int getRuleType ();

    std::string toString ();

    long getRuleID () const;

    int getHousekeepingOperation () const;

    long getEnforcementUnitID () const;

    int getEnforcementUnitType () const;

    long getEnforcementProperty () const;
};
} // namespace shepherd

#endif // SHEPHERD_HOUSEKEEPING_RULE_HPP
