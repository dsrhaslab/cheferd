
#ifndef SHEPHERD_SYSTEM_ADMIN_HPP
#define SHEPHERD_SYSTEM_ADMIN_HPP

#include "shepherd/controller/core_control_application.hpp"
#include <shepherd/utils/rules_file_parser.hpp>
#include <shepherd/session/policy_generator.hpp>

namespace shepherd {

class SystemAdmin {
private:
    ControlType m_control_type { ControlType::NOOP };

public:
    SystemAdmin (ControlType control_type);

    ~SystemAdmin ();

    void operator() (CoreControlApplication* m_control_application);


};
} // namespace shepherd

#endif // SHEPHERD_SYSTEM_ADMIN_HPP