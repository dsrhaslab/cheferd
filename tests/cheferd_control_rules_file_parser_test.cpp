/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#include <cheferd/controller/controller.hpp>
#include <cheferd/utils/logging.hpp>

namespace cheferd {

int TestControlStaticRule (const std::string path)
{
    std::vector<std::vector<std::string>> m_staged_rules {};

    std::string line;
    std::ifstream input_stream;
    input_stream.open (path);

    int total_rules = 0;

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
        return 1;
    }

    for (auto& staged_rule : m_staged_rules) {

        std::string enf_rule;

        for (char const& c : staged_rule[0]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        for (char const& c : staged_rule[1]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        for (char const& c : staged_rule[5]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        if (staged_rule[2].compare ("job") == 0) {
        } else if (staged_rule[2].compare ("user") == 0) {
        } else {
            return 1;
        }

        // Submit to queue in Control
    }
    return 0;
}

int TestControlDynamicRule (const std::string path)
{
    std::vector<std::vector<std::string>> m_staged_rules {};

    std::string line;
    std::ifstream input_stream;
    input_stream.open (path);

    int total_rules = 0;

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
        return 1;
    }

    for (auto& staged_rule : m_staged_rules) {

        for (char const& c : staged_rule[0]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        for (char const& c : staged_rule[1]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        for (char const& c : staged_rule[5]) {
            if (std::isdigit (c) != 0) {
                return 1;
            }
        }

        if (staged_rule[2].compare ("demand") != 0) {
            return 1;
        }
    }
    return 0;
}

int TestControlRules ()
{

    int result = 0;

    if (TestControlStaticRule (option_static_rules_with_time_file_path_job_op) != 0) {
        Logging::log_error ("Failed at \"job_op\" rules");
        result++;
    }
    if (TestControlStaticRule (option_static_rules_with_time_file_path_job_metadata) != 0) {
        Logging::log_error ("Failed at \"job_metadata\" rules");
        result++;
    }
    if (TestControlStaticRule (option_static_rules_with_time_file_path_job_data) != 0) {
        Logging::log_error ("Failed at \"job_data\" rules");
        result++;
    }
    if (TestControlStaticRule (option_static_rules_with_time_file_path_job) != 0) {
        Logging::log_error ("Failed at \"job\" rules");
        result++;
    }
    if (TestControlStaticRule (option_static_rules_with_time_file_path_user) != 0) {
        Logging::log_error ("Failed at \"user\" rules");
        result++;
    }
    if (TestControlDynamicRule (option_dynamic_rules_with_time_file_path_) != 0) {
        Logging::log_error ("Failed at \"dynamic\" rules");
        result++;
    }

    return 0;
}

} // namespace cheferd

int main ()
{
    int res1 = cheferd::TestControlRules ();
    return res1;
}
