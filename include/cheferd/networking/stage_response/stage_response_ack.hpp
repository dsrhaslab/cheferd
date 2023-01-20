/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_ACK_HPP
#define CHEFERD_STAGE_RESPONSE_ACK_HPP

#include "stage_response.hpp"

namespace cheferd {

/**
 * StageResponseACK class.
 */
class StageResponseACK : public StageResponse {

private:
    int ack_value_;

public:
    /**
     * StageResponseACK default constructor.
     */
    StageResponseACK ();

    /**
     * StageResponseACK parameterized constructor.
     * @param value
     */
    StageResponseACK (const int& response_type, const int& value);

    /**
     * StageResponseACK default destructor.
     */
    ~StageResponseACK () override;

    /**
     * ResponseType: ...
     * @return
     */
    int ResponseType () const override;

    /**
     * ACKValue: ...
     * @return
     */
    int ACKValue () const;

    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};

} // namespace cheferd
#endif // CHEFERD_STAGE_RESPONSE_ACK_HPP
