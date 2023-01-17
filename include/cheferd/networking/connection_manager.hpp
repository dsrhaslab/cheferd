/**
 *   Written by Ricardo Macedo and João Paulo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_CONNECTION_MANAGER_HPP
#define CHEFERD_CONNECTION_MANAGER_HPP

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>
#include <cheferd/controller/core_control_application.hpp>
#include <cheferd/controller/local_control_application.hpp>
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
 * ConnectionManager class.
 * Complete me ...
 */
class ConnectionManager {

private:
    /**
     * Operator: After establishing the connection with the data plane stage,
     * launch an ephemeral Data Plane session.
     * @param socket Socket identifier (file descriptor).
     * @param session Data plane session pointer.
     */
    void operator() (int socket, DataPlaneSession* session) {};

public:
    /**
     * ConnectionManager default destructor.
     */
    virtual ~ConnectionManager () = default;

    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Data Plane stages.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    virtual void Start (ControlApplication* application_ptr) {};

    /**
     * Start: Execute an endless loop that continuously accepts connections from
     * Local controllers.
     * TODO: refactor -- merge with the previous method; should not use
     * different methods, just use the base class.
     */
    // void Start (LocalControlApplication* application_ptr);
    virtual void Stop () {};
};
} // namespace cheferd

#endif // CHEFERD_CONNECTION_MANAGER_HPP
