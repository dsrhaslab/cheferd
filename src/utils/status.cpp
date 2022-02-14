/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/utils/status.hpp>

namespace shepherd {

PStatus::PStatus () : state_ { StatusCode::nostatus }
{ }

PStatus::~PStatus () = default;

PStatus::PStatus (PStatus::StatusCode code) : state_ { code }
{ }

PStatus PStatus::OK ()
{
    return PStatus (StatusCode::ok);
}

PStatus PStatus::NotSupported ()
{
    return PStatus (StatusCode::notsupported);
}

PStatus PStatus::Error ()
{
    return PStatus (StatusCode::error);
}

bool PStatus::isOk ()
{
    return (state_ == StatusCode::ok);
}

bool PStatus::isNotSupported ()
{
    return (state_ == StatusCode::notsupported);
}

bool PStatus::isError ()
{
    return (state_ == StatusCode::error);
}

std::string PStatus::toString ()
{
    std::string state_string;

    switch (state_) {
        case StatusCode::ok:
            state_string = "OK";
            break;

        case StatusCode::notsupported:
            state_string = "NotSupported";
            break;

        case StatusCode::error:
            state_string = "Error";
            break;

        case StatusCode::nostatus:
        default:
            state_string = "Unknown Status";
            break;
    }

    return state_string;
}

} // namespace shepherd
