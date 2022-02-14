/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_SOUTHBOUND_INTERFACE_HPP
#define SHEPHERD_SOUTHBOUND_INTERFACE_HPP

#include <shepherd/networking/interface_definitions.hpp>
#include <shepherd/utils/status.hpp>

namespace shepherd {

// Execution Types
#define EXEC_HSK 1
#define EXEC_DIF 2
#define EXEC_ENF 3

class SouthboundInterface {
public:
    virtual PStatus stage_handshake (int socket,
        ControlOperation* operation,
        StageSimplifiedHandshakeRaw& stage_handshake_obj)
        = 0;

    virtual PStatus mark_stage_ready (int socket,
        ControlOperation* operation,
        StageReadyRaw& stage_ready_obj,
        ACK& response)
        = 0;

    virtual PStatus create_housekeeping_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    virtual PStatus CreateDifferentiationRule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    virtual PStatus create_enforcement_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    virtual PStatus ExecuteHousekeepingRules (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    virtual PStatus RemoveRule (int socket, ControlOperation* operation, int rule_id, ACK& response)
        = 0;

    virtual PStatus collect_statistics (int socket, ControlOperation* operation) = 0;

    //    virtual PStatus CollectStatisticsKVS (int socket, ControlSend* send,
    //    StatsKVSRaw& response_object) = 0;
};
} // namespace shepherd

#endif // SHEPHERD_SOUTHBOUND_INTERFACE_HPP