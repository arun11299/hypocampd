#ifndef HYPOCAMPD_INET_ADDR_H
#define HYPOCAMPD_INET_ADDR_H

extern "C" {
    #include <netinet/in.h>
}
#include <string>

namespace hypocampd {

    class InetAddr {
    public:
	InetAddr() = default;
        InetAddr(const std::string& host, uint16_t port);
        /*
         * Constructor for taking in string of the format
         * <IP Addr>:<Port>
         */
        InetAddr(const std::string& host_port);

        uint32_t size() const noexcept {
            return sizeof(sockaddr_in);
        }

	void initialize(const char* host, uint16_t port);

	std::string to_string() const;

	friend bool operator== (const InetAddr& a, const InetAddr& b);
	friend bool operator< (const InetAddr& a, const InetAddr& b);

    private:
        struct sockaddr_in m_addr;         
    };

    inline
    bool operator== (const InetAddr& a, const InetAddr& b) {
        if (a.m_addr.sin_port == b.m_addr.sin_port &&
            a.m_addr.sin_addr.s_addr == b.m_addr.sin_addr.s_addr) {
            return true;
        }
        return false;
    }

    inline
    bool operator< (const InetAddr& a, const InetAddr& b) {
        if (a.m_addr.sin_addr.s_addr < b.m_addr.sin_addr.s_addr) {
            return true;
        }
        return false;
    }

}; // END namespace hypocampd

#endif
