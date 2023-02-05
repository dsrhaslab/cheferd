/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_ACK_HPP
#define CHEFERD_STAGE_RESPONSE_ACK_HPP

#include "stage_response.hpp"

namespace cheferd {

/**
 * StageResponseACK class.
 * StageResponseACK is used for simple responses that only requires one integer.
 * Currently, the StageResponseACK class contains the following variables:
 * - ack_value_: ack value according to AckCode (e.g., 1->Ok, 0->Error)
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
     * @param response_type Type of response.
     * @param value Response value.
     */
    StageResponseACK (const int& response_type, const int& value);

    /**
     * StageResponseACK default destructor.
     */
    ~StageResponseACK () override;

    /**
     * ResponseType: Get response's type.
     * @return Type of response.
     */
    int ResponseType () const override;

    /**
     * ACKValue: Get ack_value_ value.
     * @return Ack value according to AckCode (e.g., 1->Ok, 0->Error)
     */
    int ACKValue () const;

    /**
     * toString: Converts response to string.
     * @return Response in string format.
     */
    std::string toString () const override;
};

} // namespace cheferd
#endif // CHEFERD_STAGE_RESPONSE_ACK_HPP
