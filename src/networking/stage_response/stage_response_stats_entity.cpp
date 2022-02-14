/**
 *   Copyright (c) 2020 INESC TEC.
 **/

#include "shepherd/networking/stage_response/stage_response_stats_entity.hpp"

namespace shepherd {

//    StageResponseStatsEntity default constructor.
StageResponseStatsEntity::StageResponseStatsEntity ()
{ }

//    StageResponseStatsEntity parameterized constructor.
StageResponseStatsEntity::StageResponseStatsEntity (const int& response_type,
    std::unique_ptr<std::unordered_map<std::basic_string<char>, double>>& stats_ptr
    ) :
    StageResponse { response_type },
    entity_rates { std::move(stats_ptr) }
{ }

//    StageResponseStatsTF default destructor.
StageResponseStatsEntity::~StageResponseStatsEntity () = default;

//    ResponseType call. (...)
int StageResponseStatsEntity::ResponseType () const
{
    return response_type_;
}

//    toString call. (...)
std::string StageResponseStatsEntity::toString () const
{
    //std::string return_value_t = "StageResponseStatsEntity {" +
      //  std::to_string (m_metadata_total_rate / 1024 / 1024) + "}";

    return "";
}

} // namespace shepherd
