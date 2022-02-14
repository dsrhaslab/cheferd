/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STAGE_RESPONSE_ACK_HPP
#define SHEPHERD_STAGE_RESPONSE_ACK_HPP

#include "stage_response.hpp"

namespace shepherd {

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

} // namespace shepherd
#endif // SHEPHERD_STAGE_RESPONSE_ACK_HPP
