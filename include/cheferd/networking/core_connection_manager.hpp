/**
 *   Written by Ricardo Macedo and João Paulo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_CORE_CONNECTION_MANAGER_HPP
#define CHEFERD_CORE_CONNECTION_MANAGER_HPP

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>
#include <cheferd/controller/core_control_application.hpp>
#include <cheferd/networking/connection_manager.hpp>
#include <cheferd/networking/local_interface.hpp>
#include <cheferd/session/data_plane_session.hpp>
#include <cheferd/session/local_controller_session.hpp>
#include <cheferd/utils/options.hpp>
#include <cheferd/utils/status.hpp>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <thread>

#ifdef BAZEL_BUILD
#include "examples/protos/controllers_grpc_interface.grpc.pb.h"
#else
#include "controllers_grpc_interface.grpc.pb.h"
#endif

using controllers_grpc_interface::ACK;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::ConnectRequest;
using controllers_grpc_interface::StageInfoConnect;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using controllers_grpc_interface::GlobalToLocal;
using controllers_grpc_interface::LocalToGlobal;
using grpc::Channel;
using grpc::ClientContext;

namespace cheferd {

#define INITIAL_CONNECTION_DELAY 500000
#define MAX_CONNECTION_DELAY     INT_MAX

/**
 * ConnectionManager class.
 * Complete me ...
 */

class CoreConnectionManager : public LocalToGlobal::Service, public ConnectionManager {

private:
    CoreControlApplication* m_control_application_ptr;
    std::string core_address;
    int index_t;
    std::unique_ptr<Server> server;

    /**
     * Operator: After establishing the connection with the data plane stage,
     * launch an ephemeral Data Plane session.
     * @param socket Socket identifier (file descriptor).
     * @param session Data plane session pointer.
     */
    Status ConnectLocalToGlobal (ServerContext* context,
        const ConnectRequest* request,
        ConnectReply* reply) override;

    Status ConnectStageToGlobal (ServerContext* context,
        const StageInfoConnect* request,
        ConnectReply* reply) override;

public:
    /**
     * CoreConnectionManager default constructor.
     */

    /**
     *  CoreConnectionManager
     *  parameterized constructor.
     *  Initializes parameters with the configuration values based on the data
     * plane instance.
     *  @param controller_address Defines the global controller address
     */

    CoreConnectionManager (const std::string& controller_address);

    /**
     * CoreConnectionManager default destructor.
     */
    ~CoreConnectionManager ();

    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Data Plane stages.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    // void Start (CoreControlApplication* application_ptr);
    void Start (ControlApplication* application_ptr) override;

    void Stop () override;
};
} // namespace cheferd

#endif // CHEFERD_CORE_CONNECTION_MANAGER_HPP
