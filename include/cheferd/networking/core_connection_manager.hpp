/**
 *   Copyright (c) 2022 INESC TEC.
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
 * CoreConnectionManager class.
 * CoreConnectionManager is used to manage the connections to the core controller.
 * Currently, the CoreConnectionManager class contains the following variables:
 * - m_control_application_ptr: ControlApplication component.
 * - core_address: Core controller address.
 * - index_t: local controller index.
 * - server: unique_ptr of Server to send requests to core controller.
 */
class CoreConnectionManager : public LocalToGlobal::Service, public ConnectionManager {

private:
    CoreControlApplication* m_control_application_ptr;
    std::string core_address;
    int index_t;
    std::unique_ptr<Server> server;

    /**
     * ConnectLocalToGlobal: Connect local controller to core controller.
     * @param context Server context.
     * @param request Rules to be enforced.
     * @param reply Response.
     * @return Returns Status value that defines if the operation was successful.
     */
    Status ConnectLocalToGlobal (ServerContext* context,
        const ConnectRequest* request,
        ConnectReply* reply) override;

    /**
     * ConnectStageToGlobal: Connect stage to core controller.
     * @param context Server context.
     * @param request Rules to be enforced.
     * @param reply Response.
     * @return Returns Status value that defines if the operation was successful.
     */
    Status ConnectStageToGlobal (ServerContext* context,
        const StageInfoConnect* request,
        ConnectReply* reply) override;

public:
    /**
     *  CoreConnectionManager parameterized constructor.
     *  @param controller_address Core controller address
     */
    CoreConnectionManager (const std::string& controller_address);

    /**
     * CoreConnectionManager default destructor.
     */
    ~CoreConnectionManager ();

    /**
     * Start: Execute a server that continuously accepts connections.
     * @param application_ptr ControlApplication object.
     */
    void Start (ControlApplication* application_ptr) override;

    /**
     * Stop: Stop connection manager.
     */
    void Stop () override;
};
} // namespace cheferd

#endif // CHEFERD_CORE_CONNECTION_MANAGER_HPP
