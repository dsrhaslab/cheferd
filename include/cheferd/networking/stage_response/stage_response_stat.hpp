/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_STAGE_RESPONSE_STAT_HPP
#define CHEFERD_STAGE_RESPONSE_STAT_HPP

#include "stage_response.hpp"

namespace cheferd {


/**
 * StageResponseStat class.
 * StageResponseStat is used for responses that hold a single data plane stage statistic.
 * Currently, the StageResponseStat class contains the following variables:
 * - m_value: data plane stage total rate.
 */
class StageResponseStat : public StageResponse {
private:
    double m_total_rate;

public:
    /**
     * StageResponseStat default constructor.
     */
    StageResponseStat ();

    /**
     * StageResponseStat parameterized constructor.
     * @param response_type Type of response.
     * @param total_rate Data plane stage total rate.
     */
    StageResponseStat (const int& response_type, const double& total_rate);

    /**
     * StageResponseStat default destructor.
     */
    ~StageResponseStat () override;

    /**
     * ResponseType: Get response's type.
     * @return Type of response.
     */
    int ResponseType () const override;

    /**
     * get_total_rate: Get data plane stage total rate.
     * @return Data plane stage's total rate.
     */
    double get_total_rate () const;

    /**
     * toString: Converts response to string.
     * @return Response in string format.
     */
    std::string toString () const override;
};
} // namespace cheferd

#endif // CHEFERD_STAGE_RESPONSE_STAT_HPP
