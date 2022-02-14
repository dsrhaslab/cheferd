/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <archive/execute_rule.hpp>
#include <archive/housekeeping_rule.hpp>
#include <cstdio>
#include <shepherd/data_plane_session.hpp>
#include <shepherd/utils_make_unique.hpp>

namespace shepherd {

void EnqueueAndDequeue (DataPlaneSession* session)
{
    std::unique_ptr<InstanceRule> execPtr = make_unique<ExecuteRule> (1);
    std::unique_ptr<InstanceRule> hskPtr = make_unique<HSKRule> (10, 1, 10, 10, 1);

    session->EnqueueUserDefinedHousekeepingRule (RULE_HSK, std::move (hskPtr));
    session->EnqueueUserDefinedHousekeepingRule (RULE_EXEC, std::move (execPtr));

    //        Rule rule_tmp {RULE_HSK, make_unique<HSKRule> (10,1,10,10,1)};
    //
    //        auto* inst = dynamic_cast<HSKRule *>(rule_tmp.getInstanceRule());

    //        fprintf (stdout, "%s\n", inst->toString().c_str());

    fprintf (stdout, "Submission queue size : %d\n", session->getSubmissionQueueSize ());

    PAIOInterface::ControlSend send {};

    InstanceRule* rule1 = session->dequeueRule (&send);
    fprintf (stdout, "%s\n", rule1->toString ().c_str ());
    session->tryPop ();

    InstanceRule* rule2 = session->dequeueRule (&send);
    fprintf (stdout, "%s\n", rule2->toString ().c_str ());
    session->tryPop ();

    fprintf (stdout, "Submission queue size : %d\n", session->getSubmissionQueueSize ());
}

} // namespace shepherd

int main (int argc, char** argv)
{
    shepherd::DataPlaneSession session { 10 };
    shepherd::EnqueueAndDequeue (&session);
}