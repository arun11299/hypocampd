#include "consistent_hash/src/continuum.h"
#include "common/logger.h"
#include "common/config_loader.h"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace hypocampd {

    Continuum* Continuum::m_pinstance = NULL;

    // Not thread safe
    Continuum* Continuum::instance() {
	if (!m_pinstance) {
	    m_pinstance = new Continuum();
	}
	return m_pinstance;
    }

    bool Continuum::initialize_continuum()  
    {
	std::string cfg_file = m_pconfig->get_config_path() +
				"/" + m_pconfig->get_server_cfg_file();

	FINFO("Server config file is: %s", cfg_file.c_str());

	ConfigLoader serv_cfg(cfg_file);
	PropertyMap cfg = serv_cfg.get_config();

	m_cd.servers_.reserve(m_pconfig->get_num_servers() * 
			      m_pconfig->get_reserve_factor());

	for (auto& kv : cfg){

	    if (m_cd.total_servers_ > m_pconfig->get_num_servers()) {
		ERROR("More servers defined in server config file than in properties");
		return false;
	    }
   
	    uint64_t memory = 0;
	    std::stringstream ss(kv.second);
	    ss >> memory;

	    m_cd.servers_.emplace_back(InetAddr(kv.first), memory);

	    // Update continuum info
	    m_cd.total_servers_++;
	    m_cd.total_memory_ += memory;

	}

	return true;
    }


    bool Continuum::create_continuum() {

	// 1.5 times to keep some buffer. Will help in 
	// preventing reallocations many times while adding 
	// or new servers
	m_cd.points_.reserve(m_cd.total_servers_ * 
			     m_pconfig->get_points_per_server() * 
			     m_pconfig->get_reserve_factor());



	std::for_each(m_cd.servers_.begin(), m_cd.servers_.end(), [this](server_info si) {
								    this->add_points_to_continuum(si);
								  });

	// Sort the continuum
	std::sort(m_cd.points_.begin(), m_cd.points_.end(), 
				[](const continuum_point& a, const continuum_point& b) {
                                     return a.point_ < b.point_;
                                });

	std::sort(m_cd.servers_.begin(), m_cd.servers_.end(), 
				[](const server_info& a, const server_info& b) {
				     return a.serv_addr_ < b.serv_addr_;	
			        });

	return true;
    }



    void Continuum::add_points_to_continuum(const server_info& sinfo) 
    {
	float ratio = (float) sinfo.memory_ / (float) m_cd.total_memory_;
        int numhashes = floorf(ratio * m_pconfig->get_points_per_server() *
                                   m_cd.total_servers_);

        FINFO("Number of hashes computed = %d", numhashes);

        if (numhashes > m_pconfig->get_points_per_server()) {
	    WARN("Number of hashes exceeded configuration value");
            numhashes = m_pconfig->get_points_per_server();
        }

    
        for (int i = 0; i < numhashes; i++) {
	    std::stringstream ss;
	    ss << sinfo.serv_addr_.to_string() << "-" << i;

            uint32_t hash_v = m_hash(ss.str(), 0); 
            FDEBUG("Hash = %u ; Address = %s", hash_v, sinfo.serv_addr_.to_string().c_str());
    
            m_cd.points_.emplace_back(sinfo.serv_addr_, hash_v);
            m_cd.total_points_++;
        }

	return;
    }


    InetAddr Continuum::get_server(const char* key, size_t len) const {
	uint32_t hash_val = m_hash(key, len, 0);
	FINFO("get_server:: hash = %u", hash_val);
	auto it = std::lower_bound(m_cd.points_.begin(), m_cd.points_.end(), hash_val,
				   [](const continuum_point& lhs, const continuum_point& rhs) {
					return lhs.point_ < rhs.point_;
				   });	

	// If it doesnt find any element, make it the first one.
	// TODO Check if this is the right thing to do.
	if (it == m_cd.points_.end()) {
	    INFO("Lower bound no match");
	    it = m_cd.points_.begin();
	}

	FINFO("Got address: %s at point = %u", (it->addr_).to_string().c_str(), it->point_);

	return it->addr_;
    }



    InetAddr Continuum::get_server(const std::string& key) const {
	return this->get_server(key.c_str(), key.size());
    }



    bool Continuum::add_server(const std::string& host_port, 
			       uint64_t memory) {
	InetAddr addr(host_port);
	auto it = std::lower_bound(m_cd.servers_.begin(), m_cd.servers_.end(), addr,
				   [](const server_info& lhs, const server_info& rhs) {
					return lhs.serv_addr_ < rhs.serv_addr_;	
				   });

	if (it->serv_addr_ == addr) {
	    FINFO("Duplicate data. Server %s is already present in the continuum", addr.to_string().c_str());
	    return false;
	}

	server_info si;
	si.serv_addr_ = addr; si.memory_ = memory;

	m_cd.servers_.emplace_back(si);
	std::sort(m_cd.servers_.begin(), m_cd.servers_.end(), 
					 [](const server_info& a, const server_info& b) {
                                             return a.serv_addr_ < b.serv_addr_;
                                         });

	m_cd.total_servers_++;
	m_cd.total_memory_ += memory;

	// Reconstruct the continuum ring
	this->add_points_to_continuum(si);

	std::sort(m_cd.points_.begin(), m_cd.points_.end(), 
			[](const continuum_point& a, const continuum_point& b) {
			    return a.point_ < b.point_;
			});

	return true;
	
    } 


    bool Continuum::remove_server(const std::string& host_port) {
	InetAddr addr(host_port);
	uint64_t memory = 0;

	auto it = std::lower_bound(m_cd.servers_.begin(), m_cd.servers_.end(), addr,
				    [](const server_info& lhs, const server_info& rhs) {
					return lhs.serv_addr_ < rhs.serv_addr_;
				    });

	if (it->serv_addr_ == addr) {
	    memory = it->memory_;
	    // TODO: count_if preffered instead of while loop
	    // but that will take 2 iterations over the vector
	    while (true) {
		auto fit = std::find_if(m_cd.points_.begin(), m_cd.points_.end(),
					[&addr](const continuum_point& lhs) {
					    return lhs.addr_.to_string().compare(addr.to_string()) == 0 ?
						true : false ;
					});

		if (fit == m_cd.points_.end()) {
		    break;
		}
		FINFO("Removing server point %s", addr.to_string().c_str());
		m_cd.points_.erase(fit);
		m_cd.total_points_--;
	    } // end while

	    // Remove server from server list
	    m_cd.total_servers_--;
	    m_cd.total_memory_ -= it->memory_;
	    m_cd.servers_.erase(it);
	}

    }

}; // END namespace hypocampd
