/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_RULE_HPP
#define SHEPHERD_RULE_HPP

#include "instance_rule.hpp"

#include <bits/unique_ptr.h>

namespace shepherd {

#define RULE_HSK  1
#define RULE_EXEC 2

class Rule {
public:
    int rule_type_;
    std::unique_ptr<InstanceRule> rule_;

    Rule ();
    Rule (int type, std::unique_ptr<InstanceRule> instance_rule);
    Rule (Rule& rule);
    ~Rule ();

    int getRuleType () const;

    InstanceRule* getInstanceRule ();
};
} // namespace shepherd

#endif // SHEPHERD_RULE_HPP
