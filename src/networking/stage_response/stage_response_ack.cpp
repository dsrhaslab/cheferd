/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_ack.hpp"

namespace cheferd {

// StageResponseACK default constructor.
StageResponseACK::StageResponseACK () : ack_value_ { 0 }
{ }

// StageResponseACK parameterized constructor.
StageResponseACK::StageResponseACK (const int& response_type, const int& value) :
    StageResponse { response_type },
    ack_value_ { value }
{ }

// StageResponseACK default destructor.
StageResponseACK::~StageResponseACK () = default;

// ResponseType call. Get response's type.
int StageResponseACK::ResponseType () const
{
    return response_type_;
}

// ACKValue call. Get ack_value_ value.
int StageResponseACK::ACKValue () const
{
    return ack_value_;
}

// toString call. Converts response to string.
std::string StageResponseACK::toString () const
{
    std::string return_value_t = "StageResponseACK {" + std::to_string (response_type_) + ", "
        + std::to_string (ack_value_) + "}";

    return return_value_t;
}

} // namespace cheferd
