/**
 *   Copyright (c) 2022 INESC TEC.
 **/

#ifndef CHEFERD_SYSTEM_ADMIN_HPP
#define CHEFERD_SYSTEM_ADMIN_HPP

#include "cheferd/controller/core_control_application.hpp"

#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

/** SystemAdmin class.
 * The SystemAdmin mimics the behavior a system administrator, by submitting new
 * rules to the core control application.
 * Currently, the SystemAdmin class contains the following variables:
 * - m_control_type: type of control that control application imposes (e.g., STATIC,
 * DYNAMIC_VANILLA).
 */
class SystemAdmin {
private:
    ControlType m_control_type { ControlType::NOOP };

public:
    /**
     * SystemAdmin default constructor.
     * @param control_type Type of control (STATIC, DYNAMIC_VANILLA, DYNAMIC_LEFTOVER).
     */
    explicit SystemAdmin (ControlType control_type);

    /**
     * SystemAdmin default destructor.
     */
    ~SystemAdmin ();

    /**
     * operator: SystemAdmin execution.
     * Reads rules from inputted file, and submits them to control application.
     * @param m_control_application Control application to submit rules to.
     */
    void operator() (CoreControlApplication* m_control_application);
};
} // namespace cheferd

#endif // CHEFERD_SYSTEM_ADMIN_HPP