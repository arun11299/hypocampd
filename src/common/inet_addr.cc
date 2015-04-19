#include "common/inet_addr.h"
#include "common/logger.h"
#include <sstream>
#include <cstdlib>
extern "C" {
    #include <arpa/inet.h>
}

namespace hypocampd {

    InetAddr::InetAddr(const std::string& host, 
                       uint16_t port) {
        initialize(host.c_str(), port);
    }

    InetAddr::InetAddr(const std::string& host_port) {
        size_t pos = 0;
        pos = host_port.find(':');
        if (pos == std::string::npos) {
            FERROR("Invalid IP address : %s", host_port.c_str());
            return;
        }
        std::string host(host_port.substr(0, pos));
        std::string port(host_port.substr(pos+1, host_port.size()));

        FINFO("Host is: %s and port is : %s", host.c_str(), port.c_str());
        initialize(host.c_str(), atoi(port.c_str()));
    }

    inline
    void InetAddr::initialize(const char* host, uint16_t port) {
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        inet_pton(AF_INET, host, &(m_addr.sin_addr.s_addr));
    }

    std::string InetAddr::to_string() const {
	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(m_addr.sin_addr), str, INET_ADDRSTRLEN);
	std::stringstream ss;
	ss << str << ":" << m_addr.sin_port;
	return ss.str();
    }


}; // END namespace hypocampd
