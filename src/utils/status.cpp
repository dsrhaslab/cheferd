/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/utils/status.hpp>

namespace cheferd {

// PStatus default constructor.
PStatus::PStatus () : state_ { StatusCode::nostatus }
{ }

// PStatus parameterized constructor.
PStatus::PStatus (PStatus::StatusCode code) : state_ { code }
{ }

// PStatus default destructor.
PStatus::~PStatus () = default;

// OK call. Returns OK status code.
PStatus PStatus::OK ()
{
    return PStatus (StatusCode::ok);
}

// NotSupported call. Returns NotSupported status code.
PStatus PStatus::NotSupported ()
{
    return PStatus (StatusCode::notsupported);
}

// Error call. Returns Error status code.
PStatus PStatus::Error ()
{
    return PStatus (StatusCode::error);
}

// isOk: Checks if status is OK
bool PStatus::isOk ()
{
    return (state_ == StatusCode::ok);
}

// isNotSupported: Checks if status is NotSupported.
bool PStatus::isNotSupported ()
{
    return (state_ == StatusCode::notsupported);
}

// isError: Checks if status is isError.
bool PStatus::isError ()
{
    return (state_ == StatusCode::error);
}

// toString: Converts PStatus into string format.
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

} // namespace cheferd
