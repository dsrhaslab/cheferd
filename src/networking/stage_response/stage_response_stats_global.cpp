/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_stats_global.hpp"

namespace shepherd {

//    StageResponseStatsTF default constructor.
StageResponseStatsGlobal::StageResponseStatsGlobal () :
    m_read_rate { 0 },
    m_write_rate { 0 },
    m_open_rate { 0 },
    m_close_rate { 0 },
    m_getattr_rate { 0 },
    m_metadata_total_rate { 0 }
{ }

//    StageResponseStatsGlobal parameterized constructor.
StageResponseStatsGlobal::StageResponseStatsGlobal (const int& response_type,
    const double& read_rate,
    const double& write_rate,
    const double& open_rate,
    const double& close_rate,
    const double& getattr_rate,
    const double& metadata_total_rate
    ) :
    StageResponse { response_type },
    m_read_rate { read_rate },
    m_write_rate { write_rate },
    m_open_rate { open_rate },
    m_close_rate { close_rate },
    m_getattr_rate { getattr_rate },
    m_metadata_total_rate { metadata_total_rate }
{ }

//    StageResponseStatsTF default destructor.
StageResponseStatsGlobal::~StageResponseStatsGlobal () = default;

//    ResponseType call. (...)
int StageResponseStatsGlobal::ResponseType () const
{
    return response_type_;
}

// get_read_rate call. (...)
double StageResponseStatsGlobal::get_read_rate () const
{
    return this->m_read_rate;
}

// get_write_rate call. (...)
double StageResponseStatsGlobal::get_write_rate () const
{
    return this->m_write_rate;
}

// get_open_rate call. (...)
double StageResponseStatsGlobal::get_open_rate () const
{
    return this->m_open_rate;
}

// get_close_rate call. (...)
double StageResponseStatsGlobal::get_close_rate () const
{
    return this->m_close_rate;
}

// get_getattr_rate call. (...)
double StageResponseStatsGlobal::get_getattr_rate () const
{
    return this->m_getattr_rate;
}

// get_metadata_total_rate call. (...)
double StageResponseStatsGlobal::get_metadata_total_rate () const
{
    return this->m_metadata_total_rate;
}

//    toString call. (...)
std::string StageResponseStatsGlobal::toString () const
{
    std::string return_value_t = "StageResponseStatsGlobal {" +
            std::to_string (m_read_rate / 1024 / 1024) + ", " +
            std::to_string (m_write_rate / 1024 / 1024) + ", " +
            std::to_string (m_open_rate / 1024 / 1024) + ", " +
            std::to_string (m_close_rate / 1024 / 1024) + ", " +
            std::to_string (m_getattr_rate / 1024 / 1024) + ", " +
            std::to_string (m_metadata_total_rate / 1024 / 1024) + "}";

    return return_value_t;
}

} // namespace shepherd
