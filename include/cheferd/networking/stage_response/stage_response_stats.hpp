/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_STATS_HPP
#define CHEFERD_STAGE_RESPONSE_STATS_HPP

#include "stage_response.hpp"

#include <memory>
#include <unordered_map>

namespace cheferd {

/**
 * StageResponseStats class.
 * StageResponseStats is used for responses that hold statistics from multiple data plane stages.
 * Currently, the StageResponseStats class contains the following variables:
 * - m_stats_ptr: container used for mapping a data plane stage to its collected statistics.
 */
class StageResponseStats : public StageResponse {

private:
public:
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>> m_stats_ptr;

    /**
     * StageResponseStats default constructor.
     */
    StageResponseStats ();

    /**
     * StageResponseStats parameterized constructor.
     * @param response_type Type of response.
     * @param mStats_ptr Container used for mapping a data plane stage to its collected statistics.
     */
    StageResponseStats (const int& response_type,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            mStats_ptr);

    /**
     * StageResponseStats default destructor.
     */
    ~StageResponseStats () override;

    /**
     * ResponseType: Get response's type.
     * @return Type of response.
     */
    int ResponseType () const override;

    /**
     * toString: Converts response to string.
     * @return Response in string format.
     */
    std::string toString () const override;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_STATS_TF_HPP
