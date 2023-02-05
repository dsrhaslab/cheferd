/**
 *   Copyright (c) 2022 INESC TEC.
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

/**
 * ConnectionManager class.
 * The ConnectionManager class serves as base class for the connection manager component of both
 * core and local control applications.
 */
class ConnectionManager {

private:
public:
    /**
     * ConnectionManager default destructor.
     */
    virtual ~ConnectionManager () = default;

    /**
     * Start: Execute an endless loop that continuously accepts connections.
     * @param application_ptr ControlApplication object.
     */
    virtual void Start (ControlApplication* application_ptr) {};

    /**
     * Stop: Stop connection manager.
     */
    virtual void Stop () {};
};
} // namespace cheferd

#endif // CHEFERD_CONNECTION_MANAGER_HPP
