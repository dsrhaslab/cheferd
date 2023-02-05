/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_DATA_PLANE_SESSION_HPP
#define CHEFERD_DATA_PLANE_SESSION_HPP

#include "cheferd/networking/stage_response/stage_response.hpp"
#include "cheferd/networking/stage_response/stage_response_ack.hpp"
#include "cheferd/networking/stage_response/stage_response_handshake.hpp"
#include "cheferd/networking/stage_response/stage_response_stat.hpp"
#include "cheferd/networking/stage_response/stage_response_stats.hpp"

#include <cheferd/networking/paio_interface.hpp>
#include <cheferd/utils/logging.hpp>
#include <cheferd/utils/options.hpp>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <queue>
#include <unistd.h>

namespace cheferd {

/**
 * DataPlaneSession class.
 * DataPlaneSession component serves as a liaison between the LocalControlApplication
 * and the data plane stage interface. It is used for the handshake step.
 * Currently, the DataPlaneSession class contains the following variables:
 * - session_id_: session Identifier.
 * - submission_queue_: queue that holds requests to submit to the data plane stage.
 * - submission_queue_lock_:  mutex for concurrency control over submission_queue_.
 * - submission_queue_condition_: condition for submission_queue_.
 * - completion_queue_: queue that holds responses from the data plane stage.
 * - completion_queue_lock_: mutex for concurrency control over completion_queue_.
 * - completion_queue_condition_: condition for completion_queue_.
 * - working_session_: atomic bool that stores if session is active.
 * - interface_: interface to submit requests.
 * - unix_socket_: UNIX socket.
 * - server_fd_: socket file descriptor.
 * - addrlen_: address length.
 */
class DataPlaneSession {

private:
    long session_id_;
    std::queue<std::string> submission_queue_;
    std::mutex submission_queue_lock_;
    std::condition_variable submission_queue_condition_;
    std::queue<std::unique_ptr<StageResponse>> completion_queue_;
    std::mutex completion_queue_lock_;
    std::condition_variable completion_queue_condition_;
    std::atomic<bool> working_session_;
    PAIOInterface interface_;
    struct sockaddr_un unix_socket_;
    int server_fd_;
    int addrlen_;

    /**
     * PrepareUnixConnection: Prepare UNIX Domain socket connections
     * between the control plane and the data plane stage (single).
     * @param socket_name UNIX Domain Socket name.
     */
    void PrepareUnixConnection (const char* socket_name);

    /**
     * SendRule: Handle the rule to be submitted to the data plane stage.
     * @param socket Socket identifier.
     * @param rule Rule to be submitted.
     * @param operation ControlOperation.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * PStatus::Error() otherwise
     */
    PStatus SendRule (int socket, const std::string& rule, ControlOperation* operation);

    /**
     * EnqueueRuleInSubmissionQueue: Enqueue rule in the submission_queue_ in
     * string-based format.
     * @param rule Rule to be enqueued.
     */
    void EnqueueRuleInSubmissionQueue (const std::string& rule);

    /**
     * DequeueRuleFromSubmissionQueue: Dequeue rule from the submission_queue_
     * in string-based format.
     * @param rule  Rule dequeued.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * PStatus::Error() otherwise.
     */
    PStatus DequeueRuleFromSubmissionQueue (std::string& rule);

    /**
     * EnqueueResponseInCompletionQueue: Enqueue response in the completion_queue_
     * in StageResponse format.
     * @param response_object Smart pointer of a StageResponse object.
     * @return PStatus::OK() if the StageResponse was successfully enqueued,
     * PStatus::Error() otherwise.
     */
    void EnqueueResponseInCompletionQueue (std::unique_ptr<StageResponse> response_object);

    /**
     * DequeueResponseFromCompletionQueue: Dequeue response from the
     * completion_queue_ in StageResponse format.
     * @return Smart pointer of a StageResponse object.
     */
    std::unique_ptr<StageResponse> DequeueResponseFromCompletionQueue ();

    /**
     * getSubmissionQueueSize: Get the total size of the submission_queue.
     * @return Return the size of the submission_queue
     */
    int getSubmissionQueueSize ();

public:
    /**
     * DataPlaneSession parameterized constructor.
     * @param socket_name Socket name.
     */
    DataPlaneSession (const char* socket_name);

    /**
     * DataPlaneSession parameterized constructor.
     * @param id Session identifier.
     * @param socket_name Socket name.
     */
    explicit DataPlaneSession (long id, const char* socket_name);

    /**
     * DataPlaneSession default destructor.
     */
    ~DataPlaneSession ();

    /**
     * StartSession: Start session execution.
     */
    void StartSession ();

    /**
     * RemoveSession: Stop session execution.
     */
    void RemoveSession ();

    /**
     * SubmitRule: Emplace rules in the Session. This is the public
     * method that will be used by ControlApplication objects to submit
     * rules. Rules are enqueued in the submission_queue_ through the
     * EnqueueRuleInSubmissionQueue call. Concurrency control is already handled
     * in the EnqueueRuleInSubmissionQueue call (controlling concurrency here as
     * well could lead to a deadlock).
     * @param submission_rule Const value of the submission rule.
     * @return Returns PStatus::OK() if the rule was successfully enqueued,
     * PStatus::Error() otherwise. (possibly revisit this return statement).
     */
    PStatus SubmitRule (const std::string& submission_rule);

    /**
     * GetRule: Pop result objects (StageResponse) from the Session.
     * This is the public method that will be used by ControlApplication
     * objects to read received StageResponse of previously submitted requests.
     * StageResponses are dequeued from the completion_queue_ through the
     * DequeueResponseFromCompletionQueue call. Concurrency control is already
     * handled in the DequeueResponseFromCompletionQueue call (controlling
     * concurrency here as well could lead to a deadlock).
     * @return Returns smart pointer (std::unique_ptr) of a StageResponse
     * object, so the caller can unmarshall based on the Base or Derived class.
     */
    std::unique_ptr<StageResponse> GetResult ();

    /**
     * SessionIdentifier: Get session identifier.
     * @return Session identifier.
     */
    long SessionIdentifier () const;
};
} // namespace cheferd

#endif // CHEFERD_DATA_PLANE_SESSION_HPP
