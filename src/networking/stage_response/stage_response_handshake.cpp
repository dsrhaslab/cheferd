/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_handshake.hpp"

namespace cheferd {

// StageResponseHandshake default constructor.
StageResponseHandshake::StageResponseHandshake ()
{
    Logging::log_debug ("StageResponseHandshake object created.");
}

// StageResponseHandshake parameterized constructor.
StageResponseHandshake::StageResponseHandshake (const int& response_type,
    const StageSimplifiedHandshakeRaw& stage) :
    m_name { std::string (stage.m_stage_name) },
    m_env { std::to_string (stage.m_pid) },
    m_pid { stage.m_pid },
    m_ppid { stage.m_ppid },
    m_hostname { std::string (stage.m_stage_hostname) },
    m_user { std::string (stage.m_stage_user) }
{ }

// StageResponseHandshake default destructor.
StageResponseHandshake::~StageResponseHandshake () = default;

// ResponseType call. Get response's type.
int StageResponseHandshake::ResponseType () const
{
    return response_type_;
}

// get_stage_name call. Get data plane stage job's name.
std::string StageResponseHandshake::get_stage_name () const
{
    return this->m_name;
}

// get_stage_env call. Get data plane stage job's env.
std::string StageResponseHandshake::get_stage_env () const
{
    return this->m_env;
}

// get_stage_pid call. Get data plane stage pid.
pid_t StageResponseHandshake::get_stage_pid () const
{
    return this->m_pid;
}

// get_stage_ppid call. Get data plane stage ppid.
pid_t StageResponseHandshake::get_stage_ppid () const
{
    return this->m_ppid;
}

// get_stage_hostname call. Get data plane stage location.
std::string StageResponseHandshake::get_stage_hostname () const
{
    return this->m_hostname;
}

// get_stage_user call. Get data plane stage job's user.
std::string StageResponseHandshake::get_stage_user () const
{
    return this->m_user;
}

// toString call. Converts response to string.
std::string StageResponseHandshake::toString () const
{
    std::stringstream stream;
    stream << "StageInfo {";
    stream << this->m_name << ", ";
    if (!this->m_env.empty ())
        stream << this->m_env << ", ";
    if (!this->m_description.empty ())
        stream << this->m_description << ", ";
    stream << this->m_pid << ", ";
    stream << this->m_ppid << ", ";
    stream << this->m_hostname << ", ";
    stream << this->m_user << "}";

    return stream.str ();
}

} // namespace cheferd
