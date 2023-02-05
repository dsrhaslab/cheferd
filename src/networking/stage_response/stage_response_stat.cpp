/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_stat.hpp"

namespace cheferd {

// StageResponseStat default constructor.
StageResponseStat::StageResponseStat () : m_total_rate { 0 }
{ }

// StageResponseStat parameterized constructor.
StageResponseStat::StageResponseStat (const int& response_type, const double& total_rate) :
    StageResponse { response_type },
    m_total_rate { total_rate }
{ }

// StageResponseStat default destructor.
StageResponseStat::~StageResponseStat () = default;

// ResponseType call. Get response's type
int StageResponseStat::ResponseType () const
{
    return response_type_;
}

// get_total_rate call. Get data plane stage total rate
double StageResponseStat::get_total_rate () const
{
    return this->m_total_rate;
}

// toString call. Converts response to string.
std::string StageResponseStat::toString () const
{
    std::string return_value_t
        = "StageResponseStat {" + std::to_string (m_total_rate / 1024 / 1024) + "}";

    return return_value_t;
}

} // namespace cheferd
