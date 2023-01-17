//
//

#ifndef CHEFERD_COMMAND_LINE_PARSER_HPP
#define CHEFERD_COMMAND_LINE_PARSER_HPP

#include <boost/program_options.hpp>
#include <cheferd/controller/controller.hpp>
namespace po = boost::program_options;

namespace cheferd {

class CommandLineParser {

public:
    std::string config_file_path;

    void process_program_options (int argc, char** argv);

    /**
     * CommandLineParser default constructor.
     */
    CommandLineParser ();

    /**
     * CommandLineParser default destructor.
     */
    ~CommandLineParser ();
};
}; // namespace cheferd

#endif // CHEFERD_COMMAND_LINE_PARSER_HPP
