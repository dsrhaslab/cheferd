/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "archive/rule.hpp"

#include <archive/execute_rule.hpp>
#include <archive/housekeeping_rule.hpp>
#include <networking/interface_definitions.hpp>

namespace shepherd {

Rule::Rule () = default;

Rule::Rule (int type, std::unique_ptr<InstanceRule> instance_rule) :
    rule_type_ { type },
    rule_ { std::move (instance_rule) }
{ }

Rule::~Rule () = default;

int Rule::getRuleType () const
{
    return rule_type_;
}

InstanceRule* Rule::getInstanceRule ()
{
    switch (rule_type_) {
        case RULE_HSK:
            return dynamic_cast<HSKRule*> (rule_.get ());

        case RULE_EXEC:
            return dynamic_cast<ExecuteRule*> (rule_.get ());
        default:
            return nullptr;
    }
}

Rule::Rule (Rule& rule) : rule_type_ { rule.rule_type_ }, rule_ { std::move (rule.rule_) }
{ }

} // namespace shepherd