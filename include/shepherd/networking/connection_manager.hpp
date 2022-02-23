/**
 *   Written by Ricardo Macedo and João Paulo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_CONNECTION_MANAGER_HPP
#define SHEPHERD_CONNECTION_MANAGER_HPP

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>
#include <shepherd/controller/core_control_application.hpp>
#include <shepherd/controller/local_control_application.hpp>
#include <shepherd/networking/paio_interface.hpp>
#include <shepherd/session/data_plane_session.hpp>
#include <shepherd/session/local_controller_session.hpp>
#include <shepherd/utils/options.hpp>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <shepherd/utils/status.hpp>

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>

#ifdef BAZEL_BUILD
#include "examples/protos/controllers_grpc_interface.grpc.pb.h"
#else
#include "controllers_grpc_interface.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using controllers_grpc_interface::ConnectRequest;
using controllers_grpc_interface::StageInfo;
using controllers_grpc_interface::ConnectReply;
using controllers_grpc_interface::ACK;

using controllers_grpc_interface::GlobalToLocal;
using controllers_grpc_interface::LocalToGlobal;
using grpc::Channel;
using grpc::ClientContext;

namespace shepherd {

#define INITIAL_CONNECTION_DELAY 500000
#define MAX_CONNECTION_DELAY     INT_MAX

/**
 * ConnectionManager class.
 * Complete me ...
 */
class ConnectionManager final : public LocalToGlobal::Service {

private:
    struct sockaddr_in inet_socket_;
    struct sockaddr_un unix_socket_;
    int server_fd_;
    int addrlen_;

    struct sockaddr_un unix_socket_array_[option_max_connections_];
    int server_fd_array_[option_max_connections_];
    int addrlen_array_[option_max_connections_];

    CommunicationType server_type_;

    asio::thread_pool thread_pool_;

    CoreControlApplication* m_control_application_ptr;
    std::string core_address;
    int index_t;




    /**
     * Accept: Establish a new connection between the control plane and a data
     * plane stage.
     * @return Returns the assigned socket (file descriptor) of the established
     * communication.
     */
    int Accept ();

    /**
     * AcceptConnections: Establish a new connection between the control plane and a
     * data plane stage.
     * @return Returns the assigned socket (file descriptor) of the established
     * communication.
     */
    int AcceptConnections (int index);

    /**
     * Operator: After establishing the connection with the data plane stage,
     * launch an ephemeral Data Plane session.
     * @param socket Socket identifier (file descriptor).
     * @param session Data plane session pointer.
     */
    void operator() (int socket, DataPlaneSession* session);

    /**
     * PrepareInetConnection: Prepare INET-based connections between the control
     * plane and the data plane stage.
     * @param port INET connection's port.
     * @return (Change this to PStatus)
     */
    int PrepareInetConnection (int port);

    /**
     * PrepareUnixConnection: Prepare UNIX Domain socket connections between the
     * control plane and the data plane stage.
     * @param socket_name UNIX Domain Socket name.
     * @return (Change this to PStatus)
     */
    int PrepareUnixConnection (const char* socket_name, int index);

    /**
     * PrepareUnixConnections: Prepare UNIX Domain socket connections
     * between the control plane and the data plane stage.
     * @param socket_name UNIX Domain Socket name.
     * @return (Change this to PStatus)
     */
    int PrepareUnixConnections (const char* socket_name, int index);

    Status ConnectLocalToGlobal(ServerContext* context, const ConnectRequest* request,
            ConnectReply* reply);

    Status ConnectStageToGlobal(ServerContext* context, const StageInfo* request,
                ConnectReply* reply);

public:
    /**
     * ConnectionManager default constructor.
     */

    /**
     *  ConnectionManager parameterized constructor.
     *  Initializes parameters with the configuration values based on the data
     * plane instance.
     *  @param controller_address Defines the global controller address
     */

    ConnectionManager (const std::string& controller_address);

    ConnectionManager (const std::string& controller_address,  const std::string& local_address);


    /**
     * ConnectionManager default destructor.
     */
    ~ConnectionManager ();


    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Data Plane stages.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    void Start (CoreControlApplication* application_ptr);

    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Local controllers.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    void Start (LocalControlApplication* application_ptr);



};
} // namespace shepherd

#endif // SHEPHERD_CONNECTION_MANAGER_HPP
