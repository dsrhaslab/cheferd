/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_HPP
#define CHEFERD_STAGE_RESPONSE_HPP

#include <string>

namespace cheferd {

class StageResponse {
public:
    int response_type_;

    /**
     * StageResponse default constructor.
     */
    StageResponse ();

    /**
     * StageResponse parameterized constructor.
     * @param response_type
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
