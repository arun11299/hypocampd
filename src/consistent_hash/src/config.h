#ifndef HYPOCAMPD_CONFIG_H
#define HYPOCAMPD_CONFIG_H

#include <boost/noncopyable.hpp>
#include <string>
#include <cstdint>
#include "common/reference_count.h"

namespace hypocampd {

    class Config;
    using ConfigPtr = boost::intrusive_ptr<Config>;

    class Config : public SimpleRefCounter {
    public:

        static ConfigPtr instance();

        // Needs to be called before loading config
        void set_config_path(const std::string& path) {
            m_config_path = path;
        }

        bool load_config(); 
        bool reload() { return load_config(); }

        // Getters
        std::string get_config_path() const noexcept {
            return m_config_path;
        }
        std::string get_prop_cfg_file() const noexcept {
            return m_prop_config;
        }
        std::string get_server_cfg_file() const noexcept {
            return m_server_config;
        } 

        uint16_t get_num_servers() const noexcept {
            return m_num_servers;
        } 

        uint8_t get_points_per_server() const noexcept {
            return m_points_per_server;
        }

	float get_reserve_factor() const noexcept {
	    return m_reserve_factor;
	}

        void print_config();

    private:
        static ConfigPtr m_pinstance;

        uint16_t m_num_servers = 0;
        uint16_t m_points_per_server = 0;
	float m_reserve_factor = 1.5;

        std::string m_config_path;
        std::string m_prop_config = "properties.cfg";
        std::string m_server_config = "server.cfg";
    };

};

#endif
