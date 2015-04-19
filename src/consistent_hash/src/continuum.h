#ifndef HYPOCAMPD_CONTINUUM_H
#define HYPOCAMPD_CONTINUUM_H

#include <boost/noncopyable.hpp>
#include <ctime>
#include <vector>
#include "common/inet_addr.h"
#include "common/murmurhash3.h"
#include "consistent_hash/src/config.h"

namespace hypocampd {

    class Continuum: boost::noncopyable {
    public:
	struct continuum_point {
	    continuum_point() = default;
	    // Constructor is for lower bound to work.
	    // Do not make it explicit is the hint
	    continuum_point(uint32_t val): point_(val){}
	    continuum_point(InetAddr addr, uint32_t pt):
					    addr_(addr),
					    point_(pt) {}
	    
	    InetAddr addr_;
	    uint32_t point_ = 0;
	};
    
	struct server_info {
	    server_info() = default;	
	    // This constructor is for lower bound to work.
	    // Do not make this explicit is the hint
	    server_info(InetAddr addr): serv_addr_(addr){}
	    server_info(InetAddr addr, uint64_t mem):
				            serv_addr_(addr),
					    memory_(mem) {}
				
	    InetAddr serv_addr_;
	    uint64_t memory_ = 0; 
	};

	struct continuum_data {
	    time_t modified_time_;
	    uint32_t total_servers_ = 0;
	    uint64_t total_memory_  = 0; 
	    uint32_t total_points_  = 0;
	    std::vector<continuum_point> points_;
	    std::vector<server_info> servers_;
	};

	static Continuum* instance();

	bool initialize_continuum();

	bool create_continuum();

	// Getters
	uint32_t get_total_servers() const noexcept {
	    return m_cd.total_servers_; 
	}

	uint32_t get_total_points() const noexcept {
	    return m_cd.total_points_;
	}

	uint64_t get_total_memory() const noexcept {
	    return m_cd.total_memory_;
	}

	InetAddr get_server(const char* key, size_t len) const;
	InetAddr get_server(const std::string& key) const;

	bool add_server(const std::string& host_port, uint64_t memory);
	bool remove_server(const std::string& host_port);

    private:
	Continuum() {
	    m_pconfig = Config::instance();
	}

	void add_points_to_continuum(const server_info& sinfo);

	static Continuum* m_pinstance;
	continuum_data m_cd;
	MurmurHash m_hash;
	ConfigPtr m_pconfig;
    };

};

#endif
