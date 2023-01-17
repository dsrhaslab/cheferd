/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_PAIO_INTERFACE_HPP
#define CHEFERD_PAIO_INTERFACE_HPP

#include <cheferd/networking/southbound_interface.hpp>
#include <cheferd/utils/logging.hpp>
#include <cstdio>
#include <netinet/in.h>
#include <random>
#include <signal.h>
#include <sstream>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace cheferd {

/**
 * PAIOInterface class.
 * ...
 */
class PAIOInterface : public SouthboundInterface {

private:
    void parse_rule (const std::string& rule, std::vector<std::string>* tokens);

    void fill_create_channel_rule (HousekeepingCreateChannelRaw* hsk_channel_obj,
        const std::vector<std::string>& tokens);

    void fill_create_object_rule (HousekeepingCreateObjectRaw* hsk_object_obj,
        const std::vector<std::string>& tokens);

    // void fillHousekeepingAssign (HousekeepingAssignRaw* hsk_object, const
    // std::vector<std::string>& tokens);

    void fill_enforcement_rule (EnforcementRuleRaw* enf_object,
        const std::vector<std::string>& tokens);

public:
    /**
     * PAIOInterface default constructor.
     */
    PAIOInterface ();

    /**
     * PAIOInterface default destructor.
     */
    ~PAIOInterface ();

    /**
     * StageHandshake: ...
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param stage_handshake_object
     * @return
     */
    PStatus stage_handshake (int socket,
        ControlOperation* operation,
        StageSimplifiedHandshakeRaw& stage_handshake_obj) override;

    PStatus stage_handshake_address (int socket, const std::string& rule, ACK& response);

    /**
     * CreateHousekeepingRule: Create a HousekeepingRule to be installed at the
     * data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param rule ...
     * @param response ...
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus create_housekeeping_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response) override;

    /**
     * ExecuteHousekeepingRules: order to execute all pending HousekeepingRules.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param rule ...
     * @param response ...
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus ExecuteHousekeepingRules (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response) override;

    /**
     * CreateDifferentiationRule: ...
     * @param socket  ...
     * @param send  ...
     * @param rule  ...
     * @param response  ...
     * @return  ...
     */
    PStatus CreateDifferentiationRule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response) override;

    /**
     * mark_stage_ready
     * @param socket
     * @param operation
     * @param stage_ready_obj
     * @param response
     * @return
     */
    PStatus mark_stage_ready (int socket,
        ControlOperation* operation,
        StageReadyRaw& stage_ready_obj,
        ACK& response) override;

    /**
     * CreateEnforcementRule: ...
     * @param socket ...
     * @param send ...
     * @param rule ...
     * @param response ...
     * @return ...
     */
    PStatus create_enforcement_rule (int socket,
        ControlOperation* operation,
        const std::string& rule,
        ACK& response) override;

    /**
     * RemoveRule: remove a HousekeepingRule from a specific data plane stage.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param rule_id HousekeepingRule identifier.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus
    RemoveRule (int socket, ControlOperation* operation, int rule_id, ACK& response) override;

    /**
     * collect_statistics: get the statistics of a specific Enforcement Unit of
     * the current data plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_statistics (int socket, ControlOperation* operation) override;

    /**
     * CollectGlobalStatistics: get the statistics of a plane stage.
     * @param socket Corresponds to the open file descriptor/socket of a
     * specific controller-data plane communication.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return PStatus value that defines if the operation was successful.
     */
    PStatus collect_global_statistics (int socket,
        ControlOperation* operation,
        StatsGlobalRaw& stats_tf_object);
};
} // namespace cheferd

#endif // CHEFERD_PAIO_INTERFACE_HPP
