/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_stat.hpp"

namespace shepherd {

//    StageResponseStat default constructor.
StageResponseStat::StageResponseStat () :
    m_instance_read_bandwidth { 0 },
    m_instance_write_bandwidth { 0 },
    m_pid_read_bandwidth { 0 },
    m_pid_write_bandwidth { 0 }
{ }

//    StageResponseStat parameterized constructor.
StageResponseStat::StageResponseStat (const int& response_type,
    const double& instance_read_bandwidth,
    const double& instance_write_bandwidth,
    const double& pid_read_bandwidth,
    const double& pid_write_bandwidth
        ) :
    StageResponse { response_type },
    m_instance_read_bandwidth { instance_read_bandwidth },
    m_instance_write_bandwidth { instance_write_bandwidth },
    m_pid_read_bandwidth { pid_read_bandwidth },
    m_pid_write_bandwidth { pid_write_bandwidth }
{ }

//    StageResponseStat default destructor.
StageResponseStat::~StageResponseStat () = default;

//    ResponseType call. (...)
int StageResponseStat::ResponseType () const
{
    return response_type_;
}

// get_read_rate call. (...)
double StageResponseStat::get_read_rate () const
{
    return this->m_instance_read_bandwidth;
}

// get_read_rate call. (...)
double StageResponseStat::get_write_rate () const
{
    return this->m_instance_write_bandwidth;
}

// get_read_rate call. (...)
double StageResponseStat::get_pid_read_rate () const
{
    return this->m_pid_read_bandwidth;
}

// get_read_rate call. (...)
double StageResponseStat::get_pid_write_rate () const
{
    return this->m_pid_write_bandwidth;
}

//    toString call. (...)
std::string StageResponseStat::toString () const
{
    std::string return_value_t = "StatsKVS {" +
            std::to_string (m_instance_read_bandwidth / 1024 / 1024) + ", " +
            std::to_string (m_instance_write_bandwidth / 1024 / 1024) + ", " +
            std::to_string (m_pid_read_bandwidth / 1024 / 1024) + ", " +
            std::to_string (m_pid_write_bandwidth / 1024 / 1024) + "}";

    return return_value_t;
}

} // namespace shepherd
