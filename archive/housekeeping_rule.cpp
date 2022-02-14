/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "archive/housekeeping_rule.hpp"

namespace shepherd {

HSKRule::HSKRule () : rule_type_ { RULE_HSK }
{ }

HSKRule::HSKRule (long rule, int operation, long unit_id, int unit_type, long property) :
    rule_type_ { RULE_HSK },
    rule_id_ { rule },
    housekeeping_operation_ { operation },
    enforcement_unit_id_ { unit_id },
    enforcement_unit_type_ { unit_type },
    enforcement_property_ { property }
{ }

HSKRule::HSKRule (const HSKRule& rule) :
    rule_type_ { RULE_HSK },
    rule_id_ { rule.rule_id_ },
    housekeeping_operation_ { rule.housekeeping_operation_ },
    enforcement_unit_id_ { rule.enforcement_unit_id_ },
    enforcement_unit_type_ { rule.enforcement_unit_type_ },
    enforcement_property_ { rule.enforcement_property_ }
{ }

HSKRule::~HSKRule () = default;

int HSKRule::getRuleType ()
{
    return rule_type_;
}

long HSKRule::getRuleID () const
{
    return rule_id_;
}

int HSKRule::getHousekeepingOperation () const
{
    return housekeeping_operation_;
}

long HSKRule::getEnforcementUnitID () const
{
    return enforcement_unit_id_;
}

int HSKRule::getEnforcementUnitType () const
{
    return enforcement_unit_type_;
}

long HSKRule::getEnforcementProperty () const
{
    return enforcement_property_;
}

std::string HSKRule::toString ()
{
    std::string return_value = "[" + std::to_string (rule_id_) + ","
        + std::to_string (housekeeping_operation_) + "," + std::to_string (enforcement_unit_id_)
        + "," + std::to_string (enforcement_unit_type_) + ","
        + std::to_string (enforcement_property_) + "]";

    return return_value;
}

} // namespace shepherd
