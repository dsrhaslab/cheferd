/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_STAGE_RESPONSE_STATS_ENTITY_HPP
#define SHEPHERD_STAGE_RESPONSE_STATS_ENTITY_HPP

#include "stage_response.hpp"
#include <unordered_map>

namespace shepherd {

/**
 * StageResponseStatsEntity class.
 * Complete me ...
 */
class StageResponseStatsEntity : public StageResponse {
private:


public:
    /*TODO: Put this private*/
    std::unique_ptr<std::unordered_map<std::basic_string<char>, double>> entity_rates;

    /**
     * StageResponseStatsEntity default constructor.
     */
    StageResponseStatsEntity ();

    /**
     * StageResponseStatsEntity parameterized constructor.
     * @param response_type
     * @param rate_fg_tasks
     */
    StageResponseStatsEntity (const int& response_type,
        std::unique_ptr<std::unordered_map<std::basic_string<char>, double>>& stats_ptr);

    /**
     * StageResponseStatsEntity default destructor.
     */
    ~StageResponseStatsEntity () override;

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
} // namespace shepherd

#endif // SHEPHERD_STAGE_RESPONSE_STATS_ENTITY_HPP
