/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_stat.hpp"

namespace cheferd {

//    StageResponseStatsTF default constructor.
StageResponseStat::StageResponseStat () : m_total_rate { 0 }
{ }

//    StageResponseStat parameterized constructor.
StageResponseStat::StageResponseStat (const int& response_type,
    const double& total_rate) :
    StageResponse { response_type },
    m_total_rate { total_rate }
{ }

//    StageResponseStatsTF default destructor.
StageResponseStat::~StageResponseStat () = default;

//    ResponseType call. (...)
int StageResponseStat::ResponseType () const
{
    return response_type_;
}

// get_metadata_total_rate call. (...)
double StageResponseStat::get_total_rate () const
{
    return this->m_total_rate;
}

//    toString call. (...)
std::string StageResponseStat::toString () const
{
    std::string return_value_t
        = "StageResponseStat {" + std::to_string (m_total_rate / 1024 / 1024) + "}";

    return return_value_t;
}

} // namespace cheferd
