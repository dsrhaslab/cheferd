/**
 *   Written by Ricardo Macedo and João Paulo.
 *   Copyright (c) 2020 INESC TEC.
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
 * Complete me ...
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

    asio::thread_pool thread_pool_;

    int index_t;

    std::atomic<bool> working_connection_;

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

public:
    /**
     * LocalConnectionManager default constructor.
     */

    /**
     *  LocalConnectionManager parameterized constructor.
     *  Initializes parameters with the configuration values based on the data
     * plane instance.
     *  @param controller_address Defines the global controller address
     */
    LocalConnectionManager (const std::string& controller_address,
        const std::string& local_address);

    /**
     * LocalConnectionManager default destructor.
     */
    ~LocalConnectionManager ();

    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Local controllers.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    void Start (ControlApplication* application_ptr) override;

    void Stop () override;
};
} // namespace cheferd

#endif // CHEFERD_LOCAL_CONNECTION_MANAGER_HPP
