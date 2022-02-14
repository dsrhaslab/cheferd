/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_stats.hpp"

namespace shepherd {

//    StageResponseStats default constructor.
StageResponseStats::StageResponseStats ()
{ }

//    StageResponseStats parameterized constructor.
StageResponseStats::StageResponseStats (const int& response_type,
    std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<StageResponse>>>& mStats_ptr):
    StageResponse { response_type },
    m_stats_ptr{ std::move(mStats_ptr) }
{ }

//    StageResponseStats default destructor.
StageResponseStats::~StageResponseStats () = default;

//    ResponseType call. (...)
int StageResponseStats::ResponseType () const
{
    return response_type_;
}


//    toString call. (...)
std::string StageResponseStats::toString () const
{
    std::string return_value_t =  "Stats {";
    for (auto& stat : *m_stats_ptr){
        return_value_t += "[" + stat.first + ": " + stat.second->toString();
    }

    return return_value_t;
}

} // namespace shepherd
