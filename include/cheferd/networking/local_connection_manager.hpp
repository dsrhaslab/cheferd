/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_LOCAL_CONNECTION_MANAGER_HPP
#define CHEFERD_LOCAL_CONNECTION_MANAGER_HPP

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>
#include <cheferd/controller/core_control_application.hpp>
#include <cheferd/controller/local_control_application.hpp>
#include <cheferd/networking/connection_manager.hpp>
#include <cheferd/networking/paio_interface.hpp>
#include <cheferd/session/data_plane_session.hpp>
#include <cheferd/session/local_controller_session.hpp>
#include <cheferd/utils/options.hpp>
#include <cheferd/utils/status.hpp>
#include <thread>

namespace cheferd {

#define INITIAL_CONNECTION_DELAY 500000
#define MAX_CONNECTION_DELAY     INT_MAX

/**
 * LocalConnectionManager class.
 * LocalConnectionManager is used to manage the connections to the local controller.
 * Currently, the LocalConnectionManager class contains the following variables:
 * - inet_socket_: INET socket.
 * - unix_socket_: UNIX socket.
 * - server_fd_: socket file descriptor.
 * - addrlen_: address length.
 * - unix_socket_array_: container that holds sockets.
 * - server_fd_array_: container that holds file descriptors.
 * - addrlen_array_: container that holds address lengths.
 * - server_type_: connection type (e.g., UNIX, INET).
 * - index_t: data plane stage index.
 * - working_connection: atomic value that holds if connection manager is operational.
 */
class LocalConnectionManager : public ConnectionManager {

private:
    struct sockaddr_in inet_socket_;
    struct sockaddr_un unix_socket_;
    int server_fd_;
    int addrlen_;
    struct sockaddr_un unix_socket_array_[option_max_connections_];
    int server_fd_array_[option_max_connections_];
    int addrlen_array_[option_max_connections_];
    CommunicationType server_type_;
    int index_t;
    std::atomic<bool> working_connection_;

    /**
     * Accept: Establish a new connection between the control plane and a data
     * plane stage (single).
     * @return Returns the assigned socket (file descriptor) of the established
     * communication.
     */
    int Accept ();

    /**
     * AcceptConnections: Establish a new connection between the control plane and a
     * data plane stage (multiple).
     * @param index Index.
     * @return Returns the assigned socket (file descriptor) of the established
     * communication.
     */
    int AcceptConnections (int index);

    /**
     * Operator: Launch an Data Plane Session.
     * @param socket Socket identifier (file descriptor).
     * @param session Data plane session pointer.
     */
    void operator() (int socket, DataPlaneSession* session);

    /**
     * PrepareInetConnection: Prepare INET-based connections between the control
     * plane and the data plane stage.
     * @param port INET connection's port.
     * @return Returns value that defines if the operation was successful.
     */
    int PrepareInetConnection (int port);

    /**
     * PrepareUnixConnection: Prepare UNIX Domain socket connections between the
     * control plane and the data plane stage (single).
     * @param socket_name UNIX Domain Socket name.
     * @param index Index.
     * @return Returns value that defines if the operation was successful.
     */
    int PrepareUnixConnection (const char* socket_name, int index);

    /**
     * PrepareUnixConnections: Prepare UNIX Domain socket connections
     * between the control plane and the data plane stage (multiple).
     * @param socket_name UNIX Domain Socket name.
     * @param index Index.
     * @return (Change this to PStatus)
     */
    int PrepareUnixConnections (const char* socket_name, int index);

public:
    /**
     *  LocalConnectionManager parameterized constructor.
     *  @param controller_address Core controller address.
     *  @param local_address Local controller adderess.
     */
    LocalConnectionManager (const std::string& controller_address,
        const std::string& local_address);

    /**
     * LocalConnectionManager default destructor.
     */
    ~LocalConnectionManager ();

    /**
     * Start: Execute an endless loop that continuously accepts connections.
     * @param application_ptr ControlApplication object.
     */
    void Start (ControlApplication* application_ptr) override;

    /**
     * Stop: Stop connection manager.
     */
    void Stop () override;
};
} // namespace cheferd

#endif // CHEFERD_LOCAL_CONNECTION_MANAGER_HPP
