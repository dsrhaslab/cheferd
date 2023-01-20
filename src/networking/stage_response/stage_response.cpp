/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response.hpp"

namespace cheferd {

StageResponse::StageResponse () = default;

StageResponse::StageResponse (const int& response_type) : response_type_ { response_type }
{ }

StageResponse::~StageResponse () = default;

int StageResponse::ResponseType () const
{
    return -1;
}

std::string StageResponse::toString () const
{
    return "StageReponse::";
}

} // namespace cheferd
