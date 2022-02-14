/**
 *   Written by Ricardo Macedo and João Paulo.
 *   Copyright (c) 2020 INESC TEC.
 **/

#include <shepherd/controller/controller.hpp>

namespace shepherd {

Controller::Controller (ControllerType controller_type,
                        ControlType control_type,
                        std::string& core_address,
                        const uint64_t& cycle_sleep_time):
    m_connection_manager {core_address},
    m_system_admin {control_type},
    m_housekeeping_rules {}
{

    m_control_application
        = new CoreControlApplication(control_type, &m_housekeeping_rules, cycle_sleep_time);
}

Controller::Controller (ControllerType controller_type,
                        std::string& core_address,
                        std::string& local_address,
                        const uint64_t& cycle_sleep_time):
    m_connection_manager {core_address, local_address},
    m_system_admin { ControlType::NOOP },
    m_housekeeping_rules {}
{
    m_control_application = new LocalControlApplication(&m_housekeeping_rules, core_address, local_address, option_default_control_application_sleep);
}

//    Controller default destructor.
Controller::~Controller () = default;

//    RegisterHousekeepingRules call.
void Controller::RegisterHousekeepingRules (const std::string path)
{
    Logging::log_info ("Register Housekeeping Rules.");
    PolicyGenerator generator {};

    // create parser object. Rules are parsed at creation time.
    RulesFileParser file_parser { RuleType::housekeeping, path};
    int rules_size;

    // create, insert, and execute HousekeepingRules of type HSK_CREATE_CHANNEL
    std::vector<HousekeepingCreateChannelRaw> hsk_create_channel {};
    rules_size = file_parser.get_create_channel_rules (hsk_create_channel, -1);

    // convert HousekeepingRule EChannel objects to string and store them in
    // the this->m_housekeeping_rules container
    for (int i = 0; i < rules_size; i++) {
        std::string hsk_enf_channel;
        generator.convert_housekeeping_create_channel_string (hsk_create_channel.at (i),
            hsk_enf_channel);

        // store Housekeeping Rule of type HSK_CREATE_CHANNEL in string format
        this->m_housekeeping_rules.push_back(hsk_enf_channel);
    }

    // create, insert, and execute HousekeepingRules of type HSK_CREATE_OBJECT
    std::vector<HousekeepingCreateObjectRaw> hsk_create_object {};
    rules_size = file_parser.get_create_object_rules (hsk_create_object, -1);

    // convert HousekeepingRule EObject objects to string and store them in
    // the this->m_housekeeping_rules container
    for (int i = 0; i < rules_size; i++) {
        std::string hsk_enf_object;
        generator.convert_housekeeping_create_object_string (hsk_create_object.at (i),
            hsk_enf_object);

        // store Housekeeping Rule of type HSK_CREATE_OBJECT in string
        // format
        this->m_housekeeping_rules.push_back(hsk_enf_object);
    }
}


//    SpawnControlAlgorithm call.
void Controller::SpawnControlAlgorithm (const std::string& controller_type)
{
    if (controller_type.compare ("global") == 0){
        Logging::log_info ("Spawning Global Control Algorithm -- ");
        std::thread control_application_thread_t = std::thread (std::ref(dynamic_cast<CoreControlApplication&>(*m_control_application)));
        control_application_thread_t.detach ();
    }
    else {
        Logging::log_info ("Spawning Local Control Algorithm -- " );
        std::thread control_application_thread_t = std::thread (std::ref(dynamic_cast<LocalControlApplication&>(*m_control_application)));
        control_application_thread_t.detach ();
    }
}

//    SpawnControlAlgorithm call.
void Controller::SpawnSystemAdmin ()
{
    Logging::log_info ("Spawning System Admin -- ");

    std::thread system_admin_thread_t = std::thread (m_system_admin, dynamic_cast<CoreControlApplication*>(m_control_application));
    system_admin_thread_t.detach ();
}

//    Start call.
void Controller::Start (const std::string& controller_type)
{
    if (controller_type.compare ("global") == 0) {
        m_connection_manager.Start (dynamic_cast<CoreControlApplication*>(m_control_application));
    }
    else{
        m_connection_manager.Start (dynamic_cast<LocalControlApplication*>(m_control_application));
    }
}


} // namespace shepherd
