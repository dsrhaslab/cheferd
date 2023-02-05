/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include "cheferd/networking/stage_response/stage_response_stats.hpp"

namespace cheferd {

// StageResponseStats default constructor.
StageResponseStats::StageResponseStats ()
{ }

// StageResponseStats parameterized constructor.
StageResponseStats::StageResponseStats (const int& response_type,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& mStats_ptr) :
    StageResponse { response_type },
    m_stats_ptr { std::move (mStats_ptr) }
{ }

// StageResponseStats default destructor.
StageResponseStats::~StageResponseStats () = default;

// ResponseType call. Get response's type.
int StageResponseStats::ResponseType () const
{
    return response_type_;
}

// toString call. Converts response to string.
std::string StageResponseStats::toString () const
{
    std::string return_value_t = "Stats {";
    for (auto& stat : *m_stats_ptr) {
        return_value_t += "[" + stat.first + ": " + stat.second->toString ();
    }

    return return_value_t;
}

} // namespace cheferd
