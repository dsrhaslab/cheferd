/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STAGE_RESPONSE_STATS_GLOBAL_HPP
#define SHEPHERD_STAGE_RESPONSE_STATS_GLOBAL_HPP

#include "stage_response.hpp"

namespace shepherd {

/**
 * StageResponseStatsTF class.
 * Complete me ...
 */
class StageResponseStatsGlobal : public StageResponse {
private:
    double m_read_rate;
    double m_write_rate;
    double m_open_rate;
    double m_close_rate;
    double m_getattr_rate;
    double m_metadata_total_rate;

public:
    /**
     * StageResponseStatsGlobal default constructor.
     */
    StageResponseStatsGlobal ();

    /**
     * StageResponseStatsGlobal parameterized constructor.
     * @param response_type
     * @param rate_fg_tasks
     */
    StageResponseStatsGlobal (const int& response_type, const double& read_rate, const double& write_rate,
        const double& open_rate, const double& close_rate, const double& getattr_rate, const double& metadata_total_rate);

    /**
     * StageResponseStatsGlobal default destructor.
     */
    ~StageResponseStatsGlobal () override;

    /**
     * ResponseType: ...
     * @return
     */
    int ResponseType () const override;

    /**
     * RateFgTasks: ...
     * @return
     */
    double get_read_rate () const;

    double get_write_rate () const;

    double get_open_rate () const;

    double get_close_rate () const;

    double get_getattr_rate () const;

    double get_metadata_total_rate () const;


    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};
} // namespace shepherd

#endif // SHEPHERD_STAGE_RESPONSE_STATS_GLOBAL_HPP
