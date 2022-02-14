/**
 *   Written by Ricardo Macedo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#ifndef SHEPHERD_LOGGING_HPP
#define SHEPHERD_LOGGING_HPP

#include <iostream>
#include <shepherd/utils/options.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace shepherd {

/**
 * Logging class. This class contains the primitives to write logging messages
 * of the PAI/O data plane stage. Currently, the class supports log messages of
 * INFO, ERROR, and DEBUG qualifiers. Log messages can also be written to stdout
 * or file.
 */
class Logging {

private:
    std::string logger_name_;
    std::string log_file_path_;
    std::shared_ptr<spdlog::logger> logger_;

    /**
     * Enable logging debug messages.
     */
    void set_debug ();

public:
    static bool debug_enabled_;

    /**
     * Logging default constructor.
     */
    Logging ();

    /**
     * Logging parameterized constructor. If @param debug is true, the logging
     * mode is set to debug.
     * @param debug boolean value that defines if the debug is enabled or
     * disabled.
     */
    explicit Logging (bool debug);

    /**
     * Logging default destructor.
     */
    ~Logging ();

    /**
     * log_info: Log @param message with the INFO qualifier.
     * @param message log message
     */
    static void log_info (const std::string& message);

    /**
     * log_error: Log @param message with the ERROR qualifier.
     * @param message log message
     */
    static void log_error (const std::string& message);

    /**
     * log_debug: Log @param message with the DEBUG qualifier.
     * @param message log message
     */
    static void log_debug (const std::string& message);

    /**
     * is_debug_enabled: Validate if debug messages are enabled (debug_enabled_
     * instance). This function is to be used by the methods that are called the
     * most, such as SouthboundInterface calls, Agent and Core calls, etc.
     */
    static bool is_debug_enabled ();
};
} // namespace shepherd

#endif // SHEPHERD_LOGGING_HPP
