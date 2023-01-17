
#ifndef CHEFERD_SYSTEM_ADMIN_HPP
#define CHEFERD_SYSTEM_ADMIN_HPP

#include "cheferd/controller/core_control_application.hpp"

#include <cheferd/session/policy_generator.hpp>
#include <cheferd/utils/rules_file_parser.hpp>

namespace cheferd {

class SystemAdmin {
private:
    ControlType m_control_type { ControlType::NOOP };

public:
    explicit SystemAdmin (ControlType control_type);

    /**
     *
     */
    ~SystemAdmin ();

    /**
     * jlkdjlkjlsd
     * @param m_control_application
     */
    void operator() (CoreControlApplication* m_control_application);
};
} // namespace cheferd

#endif // CHEFERD_SYSTEM_ADMIN_HPP