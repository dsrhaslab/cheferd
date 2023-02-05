/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_COMMAND_LINE_PARSER_HPP
#define CHEFERD_COMMAND_LINE_PARSER_HPP

#include <cheferd/controller/controller.hpp>
#include <gflags/gflags.h>

namespace cheferd {

/**
 * CommandLineParser class.
 * CommandLineParser processes command line arguments.
 * Currently, the CommandLineParser class contains the following variables:
 * - config_file_path: path to configuration file.
 */
class CommandLineParser {

public:
    std::string config_file_path;

    /**
     * process_program_options. Process command line arguments.
     * @param argc Number of arguments.
     * @param argv Arguments.
     */
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
