/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef CHEFERD_HANDSHAKE_SESSION_HPP
#define CHEFERD_HANDSHAKE_SESSION_HPP

#include "cheferd/networking/stage_response/stage_response.hpp"
#include "cheferd/networking/stage_response/stage_response_ack.hpp"
#include "cheferd/networking/stage_response/stage_response_handshake.hpp"

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

class HandshakeSession {

private:
    long session_id_;

    std::queue<std::string> submission_queue_; // queue that contains the request to submit to the
    // data plane
    std::mutex submission_queue_lock_;

    std::queue<std::unique_ptr<StageResponse>> completion_queue_; // queue that contains the
    // responses from the data plane
    std::mutex completion_queue_lock_;
    std::condition_variable completion_queue_condition_;

    std::atomic<bool> working_session_;

    PAIOInterface interface_;

    /**
     * SendRule: handle the rule to be submitted to the data plane stage.
     * @Private
     * @param socket Corresponds to the opened file descriptor/socket of a
     * specific controller-data plane communication.
     * @param send ControlSend object that contains the type of rule that will
     * be sent, its size, and the id.
     * @return Returns a PStatus value, indicating whether or not the operation
     * was successful.
     */
    PStatus SendRule (int socket, const std::string& rule, ControlOperation* operation);

    /**
     * EnqueueRuleInSubmissionQueue: enqueue rule in the submission_queue_ in
     * string-based format.
     * @param rule Const reference of a rule. Rules can be of Housekeeping,
     * Differentiation, or Enforcement types.
     * @return PStatus::OK() if the rule was successfully enqueued,
     * PStatus::Error() otherwise.
     */
    void EnqueueRuleInSubmissionQueue (const std::string& rule);

    /**
     * DequeueRuleFromSubmissionQueue: dequeue rule from the submission_queue_
     * in string-based format.
     * @param rule Reference of a rule for the dequeued rule to be placed.
     * @return PStatus::OK() if the rule was successfully dequeued,
     * PStatus::Error() otherwise.
     */
    PStatus DequeueRuleFromSubmissionQueue (std::string& rule);

    /**
     * EnqueueResponseInCompletionQueue: enqueue response in the
     * completion_queue_ in StageResponse format.
     * @param response_object Smart pointer (std::unique) of a StageResponse
     * object.
     * @return PStatus::OK() if the StageResponse was successfully enqueued,
     * PStatus::Error() otherwise.
     */
    void EnqueueResponseInCompletionQueue (std::unique_ptr<StageResponse> response_object);

    /**
     * DequeueResponseFromCompletionQueue: dequeue response from the
     * completion_queue_ in StageResponse format.
     * @return Returns smart pointer (std::unique_ptr) of a StageResponse
     * object.
     */
    std::unique_ptr<StageResponse> DequeueResponseFromCompletionQueue ();

    /**
     * getSubmissionQueueSize: get the total size of the submission_queue.
     * @Private
     * @return return the size of the submission_queue
     */
    int getSubmissionQueueSize ();

public:
    std::condition_variable submission_queue_condition_;

    /**
     * HandshakeSession default constructor.
     */
    HandshakeSession ();

    /**
     * HandshakeSession parameterized constructor.
     * @param id Data plane stage session id.
     */
    explicit HandshakeSession (long id);

    /**
     * HandshakeSession default destructor.
     */
    ~HandshakeSession ();

    /**
     * StartSession: begin the enforcement session between the controller and
     * the data plane stage.
     * @param socket Socket identifier of the communication channel between the
     * controller and the data plane stage.
     */
    void StartSession (int socket);

    void RemoveSession ();

    /**
     * SubmitRule: emplace rules in the Session. This is the public
     * method that will be used by ControlApplicationKVS objects to submit
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
     * GetRule: pop result objects (StageResponse) from the Session.
     * This is the public method that will be used by ControlApplicationKVS
     * objects to read received StageResponse of previously submitted requests.
     * StageResponses are dequeued from the completion_queue_ through the
     * DequeueResponseFromCompletionQueue call. Concurrency control is already
     * handled in the DequeueResponseFromCompletionQueue call (controlling
     * concurrency here as well could lead to a deadlock).
     * @return Returns smart pointer (std::unique_ptr) of a StageResponse
     * object, so the caller can unmarshall based on the Base or Derived class.
     */
    std::unique_ptr<StageResponse> GetResult ();

    long SessionIdentifier () const;
};
} // namespace cheferd

#endif // CHEFERD_HANDSHAKE_SESSION_HPP
