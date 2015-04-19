#include "common/inet_addr.h"
#include <iostream>

using namespace hypocampd;

int main() {
    InetAddr addr("10.1.9.6:8080");
    InetAddr addr1("10.3.4.5", 567);

    std::cout << addr1.to_string() << std::endl;

    if (addr == addr1) {}
    
    return 0;
}
