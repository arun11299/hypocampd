#include "consistent_hash/src/config.h"

using namespace hypocampd;

int main() {
    Config::instance()->set_config_path("/home/amuralidharan/dev/hypocampd/src/consistent_hash/config");
    Config::instance()->load_config();
    Config::instance()->print_config();

    return 0;
}
