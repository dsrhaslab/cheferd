/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STAGE_RESPONSE_STAT_HPP
#define SHEPHERD_STAGE_RESPONSE_STAT_HPP

#include "stage_response.hpp"

namespace shepherd {

/**
 * StageResponseStat class.
 * Complete me ...
 */
class StageResponseStat : public StageResponse {

private:
    double m_instance_read_bandwidth;
    double m_instance_write_bandwidth;
    double m_pid_read_bandwidth;
    double m_pid_write_bandwidth;

public:
    /**
     * StageResponseStat default constructor.
     */
    StageResponseStat ();

    /**
     * StageResponseStat parameterized constructor.
     * @param response_type
     * @param rate_fg_tasks
     */
    StageResponseStat (const int& response_type,
        const double& instance_read_bandwidth,
        const double& instance_write_bandwidth,
        const double& pid_read_bandwidth,
        const double& pid_write_bandwidth
        );

    /**
     * StageResponseStat default destructor.
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
    double get_read_rate () const;


    double get_write_rate () const;


    double get_pid_read_rate () const;


    double get_pid_write_rate () const;

    /**
     * toString: ...
     * @return
     */
    std::string toString () const override;
};
} // namespace shepherd

#endif // SHEPHERD_STAGE_RESPONSE_STATS_TF_HPP
