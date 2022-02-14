/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <archive/instance_rule.hpp>

namespace shepherd {

InstanceRule::InstanceRule () = default;

InstanceRule::~InstanceRule () = default;

int InstanceRule::getRuleType ()
{
    return 0;
}

std::string InstanceRule::toString ()
{
    return "";
}
} // namespace shepherd