/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_ack.hpp"

namespace shepherd {

StageResponseACK::StageResponseACK () : ack_value_ { 0 }
{ }

StageResponseACK::StageResponseACK (const int& response_type, const int& value) :
    StageResponse { response_type },
    ack_value_ { value }
{ }

StageResponseACK::~StageResponseACK () = default;

int StageResponseACK::ResponseType () const
{
    return response_type_;
}

int StageResponseACK::ACKValue () const
{
    return ack_value_;
}

std::string StageResponseACK::toString () const
{
    std::string return_value_t = "StageResponseACK {" + std::to_string (response_type_) + ", "
        + std::to_string (ack_value_) + "}";

    return return_value_t;
}

} // namespace shepherd
