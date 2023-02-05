/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_SOUTHBOUND_INTERFACE_HPP
#define CHEFERD_SOUTHBOUND_INTERFACE_HPP

#include <cheferd/networking/interface_definitions.hpp>
#include <cheferd/utils/status.hpp>

namespace cheferd {

/**
 * SouthboundInterface class.
 * Base class that defines the interface to communicate with a PAIO data plane stage.
 */
class SouthboundInterface {
public:
    /**
     * stage_handshake: Handshake a data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id..
     * @param stage_handshake_obj StageSimplifiedHandshakeRaw object stores data plane stage
     * detailed information.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * * PStatus::Error() otherwise.
     */
    virtual PStatus stage_handshake (int socket,
        ControlOperation* operation,
        StageSimplifiedHandshakeRaw& stage_handshake_obj)
        = 0;

    /**
     * mark_stage_ready: Mark data plane stage as ready.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id..
     * @param stage_ready_obj StageReadyRaw object stores if a data plane stage
     * is ready
     * @param response Response obtained.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * * PStatus::Error() otherwise.
     */
    virtual PStatus mark_stage_ready (int socket,
        ControlOperation* operation,
        StageReadyRaw& stage_ready_obj,
        ACK& response)
        = 0;

    /**
     * create_housekeeping_rule: Creates a housekeeping rule at the data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id..
     * @param rule Housekeeping rule to be created.
     * @param response Response obtained.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * * PStatus::Error() otherwise.
     */
    virtual PStatus create_housekeeping_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    /**
     * create_enforcement_rule: Creates an enforcement rule at the data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id..
     * @param rule Enforcement rule to be created.
     * @param response Response obtained.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * * PStatus::Error() otherwise.
     */
    virtual PStatus create_enforcement_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response)
        = 0;

    /**
     * RemoveRule: Remove a HousekeepingRule from a specific data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param rule_id HousekeepingRule identifier.
     * @param response Response obtained.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * PStatus::Error() otherwise.
     */
    virtual PStatus RemoveRule (int socket, ControlOperation* operation, int rule_id, ACK& response)
        = 0;

    /**
     * collect_statistics: Get the statistics of a current data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param operation ControlOperation object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    virtual PStatus collect_statistics (int socket, ControlOperation* operation) = 0;
};
} // namespace cheferd

#endif // CHEFERD_SOUTHBOUND_INTERFACE_HPP
