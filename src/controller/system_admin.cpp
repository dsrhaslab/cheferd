#include "shepherd/controller/system_admin.hpp"


namespace shepherd {

SystemAdmin::SystemAdmin (ControlType control_type):
    m_control_type {control_type}
{ }



SystemAdmin::~SystemAdmin ()
{ }




void SystemAdmin::operator() (CoreControlApplication* m_control_application)
{

    std::vector<std::vector<std::string>> m_staged_rules {};

    std::string line;
    std::ifstream input_stream;

    int total_rules = 0;
    // open file stream

    switch (m_control_type){
        case ControlType::STATIC: {
            input_stream.open (option_static_rules_with_time_file_path_job);
            break;
        }
        case ControlType::DYNAMIC: {
            input_stream.open (option_dynamic_rules_with_time_file_path_);
            break;
        }
        case ControlType::MDS: {
            input_stream.open (option_mds_rules_with_time_file_path_);
            break;
        }
        default:
            break;
    }

    // verify if stream is open
    if (input_stream.is_open ()) {
        // read line and store in line_t
        while (std::getline (input_stream, line)) {
            // send to parser and store in the respective structure
            std::vector<std::string> tokens {};
            // parse line
            size_t start;
            size_t end = 0;

            while ((start = line.find_first_not_of (' ', end)) != std::string::npos) {
                end = line.find (' ', start);
                tokens.push_back (line.substr (start, end - start));
            }

            // store parsed tokens
            m_staged_rules.push_back (tokens);
            total_rules++;
        }

        // close file stream
        input_stream.close ();
    } else {
        Logging::log_error ("SystemAdmin: cannot open file.");
    }


    auto start_time = std::chrono::system_clock::now();
    auto waitUntil = std::chrono::system_clock::now();

    for (auto& staged_rule : m_staged_rules) {

         std::string enf_rule;

         enf_rule += "|"+
             staged_rule[0] + "|" +
             staged_rule[2] + "|" +
             staged_rule[3] + "|" +
             staged_rule[4] + "|" +
             staged_rule[5] + "|";


        waitUntil = start_time + std::chrono::milliseconds(std::stoll(staged_rule[1])*1000);

        //Check time
        while (waitUntil > std::chrono::system_clock::now())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        //Submit to queue in Control

        Logging::log_debug ("SystemAdmin: Rule submitted "+ enf_rule);
        m_control_application->EnqueueRuleInQueue(enf_rule);


    }
}

}