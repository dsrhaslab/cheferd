/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <cheferd/utils/logging.hpp>

namespace cheferd {

//    static variable debug_enabled
bool Logging::debug_enabled_ = false;

//    Logging default constructor.
Logging::Logging () :
    logger_name_ { "basic_logger" },
    log_file_path_ { "/tmp/cheferd_info.txt" },
    logger_ {}
{ }

//    Logging parameterized constructor.
Logging::Logging (bool debug) :
    logger_name_ { "basic_logger" },
    log_file_path_ { "/tmp/cheferd_info.txt" },
    logger_ {}
{
    if (debug) {
        set_debug ();
    }
}

//    Logging default destructor.
Logging::~Logging () = default;

//    set_debug call. Enabled debug messages.
void Logging::set_debug ()
{
    spdlog::set_level (spdlog::level::debug);
    debug_enabled_ = true;
}

//    log_info call. Log message with the info qualifier.
void Logging::log_info (const std::string& message)
{
    spdlog::info (message);
}

//    log_error call. Log message with the error qualifier.
void Logging::log_error (const std::string& message)
{
    spdlog::error (message);
}

//    log_debug call. Log message with the debug qualifier.
void Logging::log_debug (const std::string& message)
{
    spdlog::debug (message);
}

//    is_debug_enabled call. Validate if debug messages are enabled.
bool Logging::is_debug_enabled ()
{
    return debug_enabled_;
}

} // namespace cheferd
