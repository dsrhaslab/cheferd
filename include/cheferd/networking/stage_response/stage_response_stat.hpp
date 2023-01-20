/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_STAT_HPP
#define CHEFERD_STAGE_RESPONSE_STAT_HPP

#include "stage_response.hpp"

namespace cheferd {

/**
 * StageResponseStatsTF class.
 * Complete me ...
 */
class StageResponseStat : public StageResponse {
private:
    double m_total_rate;

public:
    /**
     * StageResponseStatsGlobal default constructor.
     */
    StageResponseStat ();

    /**
     * StageResponseStatsGlobal parameterized constructor.
     * @param response_type
     * @param rate_fg_tasks
     */
    StageResponseStat (const int& response_type, const double& total_rate);

    /**
     * StageResponseStatsGlobal default destructor.
     */
    ~StageResponseStat () override;

    /**
     * ResponseType: ...
     * @return
     */
    int ResponseType () const override;

    /**
     * RateFgTasks: ...
     * @return
     */
    double get_total_rate () const;

    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_STAT_HPP
