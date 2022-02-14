/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STAGE_RESPONSE_HANDSHAKE_HPP
#define SHEPHERD_STAGE_RESPONSE_HANDSHAKE_HPP

#include "stage_response.hpp"
#include "shepherd/utils/logging.hpp"
#include "shepherd/networking/interface_definitions.hpp"
#include <sstream>

namespace shepherd {

class StageResponseHandshake : public StageResponse {

private:
    std::string m_name { "data-plane-stage" };
    std::string m_env { "env" };
    std::string m_description {};
    pid_t m_pid { 0 };
    pid_t m_ppid { 0 };
    std::string m_hostname { "hostname" };
    std::string m_user { "user" };

public:
    /**
     * StageResponseHandshake default constructor.
     */
    StageResponseHandshake ();


    StageResponseHandshake (const int& response_type, const StageSimplifiedHandshakeRaw& stage);

    /**
     * StageResponseHandshake default destructor.
     */
    ~StageResponseHandshake () override;

    /**
     * ResponseType: ...
     * @return
     */
    int ResponseType () const override;

    /**
     * StageIdentifier: ...
     * @return
     */
    std::string get_stage_name () const;

    std::string get_stage_env () const;

    pid_t get_stage_pid () const;

    pid_t get_stage_ppid () const;

    std::string get_stage_hostname () const;

    std::string get_stage_user () const;

    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};
} // namespace shepherd

#endif // SHEPHERD_STAGE_RESPONSE_HANDSHAKE_HPP
