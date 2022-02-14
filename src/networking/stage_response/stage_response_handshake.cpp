/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_handshake.hpp"

namespace shepherd {

//    StageResponseHandshake default constructor.
StageResponseHandshake::StageResponseHandshake ()
{
    Logging::log_debug ("StageResponseHandshake object created.");
}

StageResponseHandshake::StageResponseHandshake (const int& response_type, const StageSimplifiedHandshakeRaw& stage) :
    m_name { std::string (stage.m_stage_name) },
    m_env { std::string (stage.m_stage_env) },
    m_pid { stage.m_pid },
    m_ppid { stage.m_ppid },
    m_hostname { std::string (stage.m_stage_hostname) },
    m_user { std::string (stage.m_stage_user) }
{ }

//    StageResponseHandshake default destructor.
StageResponseHandshake::~StageResponseHandshake () = default;

//    ResponseType call. ...
int StageResponseHandshake::ResponseType () const
{
    return response_type_;
}

std::string StageResponseHandshake::get_stage_name () const
{
    return this->m_name;
}

std::string StageResponseHandshake::get_stage_env () const
{
    return this->m_env;
}

//    StagePid call. ...
pid_t StageResponseHandshake::get_stage_pid () const
{
    return this->m_pid;
}

pid_t StageResponseHandshake::get_stage_ppid () const
{
    return this->m_ppid;
}

std::string StageResponseHandshake::get_stage_hostname () const
{
    return this->m_hostname;
}

std::string StageResponseHandshake::get_stage_user () const
{
    return this->m_user;
}

//    toString call. ...
std::string StageResponseHandshake::toString () const
{
    std::stringstream stream;
    stream << "StageInfo {";
    stream << this->m_name << ", ";
    if (!this->m_env.empty()) stream << this->m_env << ", ";
    if (!this->m_description.empty()) stream << this->m_description << ", ";
    stream << this->m_pid << ", ";
    stream << this->m_ppid << ", ";
    stream << this->m_hostname << ", ";
    stream << this->m_user << "}";

    return stream.str ();
}

} // namespace shepherd
