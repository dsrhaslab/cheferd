/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_HANDSHAKE_HPP
#define CHEFERD_STAGE_RESPONSE_HANDSHAKE_HPP

#include "cheferd/networking/interface_definitions.hpp"
#include "cheferd/utils/logging.hpp"
#include "stage_response.hpp"

#include <sstream>

namespace cheferd {

/**
 * StageResponseHandshake class.
 * StageResponseHandshake is used for responses that hold detailed information about
 * a data plane stage.
 * Currently, the StageResponseHandshake class contains the following variables:
 * - m_value: data plane stage job's name.
 * - m_env:  data plane stage job's env.
 * - m_description:  data plane stage job's description.
 * - m_pid: data plane stage pid.
 * - m_ppid: data plane stage ppid.
 * - m_hostname: data plane stage location.
 * - m_user: data plane stage job's user.
 */
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

    /**
     * StageResponseHandshake parameterized constructor.
     * @param response_type Type of response.
     * @param stage Data plane stage detailed information.
     */
    StageResponseHandshake (const int& response_type, const StageSimplifiedHandshakeRaw& stage);

    /**
     * StageResponseHandshake default destructor.
     */
    ~StageResponseHandshake () override;

    /**
     * ResponseType: Get response's type.
     * @return Type of response.
     */
    int ResponseType () const override;

    /**
     * get_stage_name: Get data plane stage job's name.
     * @return Data plane stage job's name.
     */
    std::string get_stage_name () const;

    /**
     * get_stage_env: Get data plane stage job's env.
     * @return Data plane stage job's env.
     */
    std::string get_stage_env () const;

    /**
     * get_stage_pid: Get data plane stage pid.
     * @return Data plane stage pid.
     */
    pid_t get_stage_pid () const;

    /**
     * get_stage_ppid: Get data plane stage ppid.
     * @return Data plane stage ppid.
     */
    pid_t get_stage_ppid () const;

    /**
     * get_stage_hostname: Get data plane stage location.
     * @return Data plane stage location.
     */
    std::string get_stage_hostname () const;

    /**
     * get_stage_user: Get data plane stage job's user.
     * @return Data plane stage job's user.
     */
    std::string get_stage_user () const;

    /**
     * toString: Converts response to string.
     * @return Response in string format.
     */
    std::string toString () const override;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_HANDSHAKE_HPP
