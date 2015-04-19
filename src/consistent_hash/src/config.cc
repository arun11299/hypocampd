#include <sstream>
#include "consistent_hash/src/config.h"
#include "common/logger.h"
#include "common/config_loader.h"

namespace hypocampd {

    ConfigPtr Config::m_pinstance = NULL;

    ConfigPtr Config::instance() {
        if (!m_pinstance) {
            m_pinstance = new Config;
        }
        return m_pinstance;
    }

    bool Config::load_config() {
        std::string file = m_config_path + "/" + m_prop_config;
        FINFO("Config file loc = %s", file.c_str());

	ConfigLoader cfg_ldr(file);
	PropertyMap cfg = cfg_ldr.get_config();

	auto it = cfg.find("TOTAL_SERVERS");
	if (it == cfg.end()) {
	    ERROR("Property 'TOTAL_SERVERS' not found");
	} else {
	    std::stringstream ss(it->second);
	    ss >> m_num_servers;
	}

	it = cfg.find("POINTS_PER_SERVER");
	if (it == cfg.end()) {
	    ERROR("Property 'POINTS_PER_SERVER' not found");
	} else {
	    std::stringstream ss(it->second);
	    ss >> m_points_per_server;
	}

	it = cfg.find("RESERVE_FACTOR");
	if (it == cfg.end()) {
	    m_reserve_factor = 1.5;
	} else {
	    std::stringstream ss(it->second);
	    ss >> m_reserve_factor;
	}

    }


    void Config::print_config() {
        FINFO("No. servers = %d", m_num_servers);
        FINFO("Points per server = %d", m_points_per_server) ;
	FINFO("Reserve memory factor = %f", m_reserve_factor);
    }

};
