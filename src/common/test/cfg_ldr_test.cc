#include "common/config_loader.h"
#include <iostream>

using namespace hypocampd;

int main() {
    ConfigLoader cl("test_file.txt");
    PropertyMap props = cl.get_config(); 
    cl.print_map();
    auto it = props.find("CONFIG");
    if (it != props.end()) {
	std::cout << it->second << std::endl;
    }
    return 0;
}
