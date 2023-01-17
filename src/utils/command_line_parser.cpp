//
//

#include "cheferd/utils/command_line_parser.hpp"

namespace cheferd {

// CommandLineParser default constructor.
CommandLineParser::CommandLineParser ()
{
    Logging::log_debug ("RulesFileParser default constructor.");
}

// CommandLineParser default destructor.
CommandLineParser::~CommandLineParser ()
{
    Logging::log_debug ("CommandLineParser default destructor.");
}

void CommandLineParser::process_program_options (int argc, char** argv)
{
    po::options_description description ("Control Plane Controller Usage");

    std::string file_path;

    description.add_options () ("help", "Display this help message.") ("config_file",
        po::value<std::string> (&file_path),
        "Define config's file path.");

    po::variables_map vm;
    po::store (po::command_line_parser (argc, argv).options (description).run (), vm);
    po::notify (vm);

    if (vm.count ("help")) {
        std::cout << description << "\n";
    }

    if (vm.count ("config_file")) {
        config_file_path = file_path;
    }
}

} // namespace cheferd
