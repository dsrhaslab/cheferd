/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_STATS_HPP
#define CHEFERD_STAGE_RESPONSE_STATS_HPP

#include "stage_response_stat.hpp"

#include <memory>
#include <unordered_map>

namespace cheferd {

/**
 * StageResponseStats class.
 * Complete me ...
 */
class StageResponseStats : public StageResponse {

private:
public:
    /*TODO: Put this private*/
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>> m_stats_ptr;

    /**
     * StageResponseStats default constructor.
     */
    StageResponseStats ();

    /**
     * StageResponseStats parameterized constructor.
     * @param response_type
     * @param rate_fg_tasks
     */

    StageResponseStats (const int& response_type,
        std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>&
            mStats_ptr);

    /**
     * StageResponseStats default destructor.
     */
    ~StageResponseStats () override;

    /**
     * ResponseType: ...
     * @return
     */
    int ResponseType () const override;

    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_STATS_TF_HPP
