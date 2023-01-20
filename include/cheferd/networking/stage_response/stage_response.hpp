/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_HPP
#define CHEFERD_STAGE_RESPONSE_HPP

#include <string>

namespace cheferd {

/**
 * StageResponse class.
 * Base class for all responses from both data plane stages and local controllers.
 */
class StageResponse {
public:
    /**
     * Type of response. (e.g., STAGE_HANDSHAKE, CREATE_ENF_RULE, COLLECT_GLOBAL_STATS, ...).
     */
    int response_type_;

    /**
     * StageResponse default constructor.
     */
    StageResponse ();

    /**
     * StageResponse parameterized constructor.
     * @param response_type Type of response
     */
    StageResponse (const int& response_type);

    /**
     * StageResponse default destructor.
     */
    virtual ~StageResponse ();

    /**
     * ResponseType: ...
     * @return
     */
    virtual int ResponseType () const;

    /**
     * toString: ...
     * @return
     */
    virtual std::string toString () const;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_HPP
