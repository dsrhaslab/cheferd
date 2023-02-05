/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_LOGGING_HPP
#define CHEFERD_LOGGING_HPP

#include <cheferd/utils/options.hpp>
#include <iostream>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace cheferd {

/**
 * Logging class. This class contains the primitives to write logging messages
 * of the PAI/O data plane stage. Currently, the class supports log messages of
 * INFO, ERROR, and DEBUG qualifiers. Log messages can also be written to stdout
 * or file.
 * Currently, the Logging class contains the following variables:
 * - debug_enabled_: bool that determines if debug is enabled.
 */
class Logging {

private:
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
     * Logging parameterized constructor.
     * @param debug Boolean value that defines if the debug is enabled or disabled.
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
     * instance).
     */
    static bool is_debug_enabled ();
};
} // namespace cheferd

#endif // CHEFERD_LOGGING_HPP
