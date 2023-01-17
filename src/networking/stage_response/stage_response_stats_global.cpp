/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_stats_global.hpp"

namespace cheferd {

//    StageResponseStatsTF default constructor.
StageResponseStatsGlobal::StageResponseStatsGlobal () : m_total_rate { 0 }
{ }

//    StageResponseStatsGlobal parameterized constructor.
StageResponseStatsGlobal::StageResponseStatsGlobal (const int& response_type,
    const double& total_rate) :
    StageResponse { response_type },
    m_total_rate { total_rate }
{ }

//    StageResponseStatsTF default destructor.
StageResponseStatsGlobal::~StageResponseStatsGlobal () = default;

//    ResponseType call. (...)
int StageResponseStatsGlobal::ResponseType () const
{
    return response_type_;
}

// get_metadata_total_rate call. (...)
double StageResponseStatsGlobal::get_total_rate () const
{
    return this->m_total_rate;
}

//    toString call. (...)
std::string StageResponseStatsGlobal::toString () const
{
    std::string return_value_t
        = "StageResponseStatsGlobal {" + std::to_string (m_total_rate / 1024 / 1024) + "}";

    return return_value_t;
}

} // namespace cheferd
