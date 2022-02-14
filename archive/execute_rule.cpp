/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "archive/execute_rule.hpp"

namespace shepherd {

ExecuteRule::ExecuteRule () : rule_type_ { RULE_EXEC }
{ }

ExecuteRule::ExecuteRule (int execute_type) :
    rule_type_ { RULE_EXEC },
    execute_type_ { execute_type }
{ }

ExecuteRule::ExecuteRule (const ExecuteRule& rule) :
    rule_type_ { rule.rule_type_ },
    execute_type_ { rule.execute_type_ }
{ }

ExecuteRule::~ExecuteRule () = default;

int ExecuteRule::getRuleType ()
{
    return rule_type_;
}

std::string ExecuteRule::toString ()
{
    std::string return_value = "[" + std::to_string (execute_type_) + "]";

    return return_value;
}

} // namespace shepherd