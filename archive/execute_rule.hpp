/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_EXECUTE_RULE_HPP
#define SHEPHERD_EXECUTE_RULE_HPP

#include "instance_rule.hpp"
#include "rule.hpp"

namespace shepherd {

class ExecuteRule : public InstanceRule {
private:
    int rule_type_;
    int execute_type_;

public:
    ExecuteRule ();
    ExecuteRule (int execute_type);
    ExecuteRule (const ExecuteRule& rule);
    ~ExecuteRule ();

    int getRuleType ();

    std::string toString ();
};
} // namespace shepherd

#endif // SHEPHERD_EXECUTE_RULE_HPP
