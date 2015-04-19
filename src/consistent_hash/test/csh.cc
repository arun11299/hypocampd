#include "consistent_hash/src/continuum.h"
#include "consistent_hash/src/config.h"
#include <iostream>


// g++ -std=c++11 -I /home/amuralidharan/dev/hypocampd/src -o csh csh.cc ../src/config.cc ../src/continuum.cc ../../common/logger.cc ../../common/murmurhash3.cc ../../common/inet_addr.cc

using namespace hypocampd;

int main() {
    Config::instance()->set_config_path("/home/amuralidharan/dev/hypocampd/src/consistent_hash/config");
    Config::instance()->load_config();

    Continuum::instance()->initialize_continuum();
    Continuum::instance()->create_continuum();

    InetAddr addr = Continuum::instance()->get_server("Answ");
    addr = Continuum::instance()->get_server("Answer1");
    addr = Continuum::instance()->get_server("Answer2");
    addr = Continuum::instance()->get_server("Answer3");
    addr = Continuum::instance()->get_server("WAAnswer3");

    Continuum::instance()->add_server("10.56.3.45:443", 256);
    Continuum::instance()->add_server("10.56.3.45:443", 56);

    Continuum::instance()->remove_server("10.56.3.45:443");

    return 0;
}
