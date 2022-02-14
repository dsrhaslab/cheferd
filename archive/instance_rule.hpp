/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_INSTANCE_RULE_HPP
#define SHEPHERD_INSTANCE_RULE_HPP

#include <string>

namespace shepherd {

class InstanceRule {
public:
    InstanceRule ();
    virtual ~InstanceRule ();

    virtual int getRuleType ();

    virtual std::string toString ();
};
} // namespace shepherd

#endif // SHEPHERD_INSTANCE_RULE_HPP
