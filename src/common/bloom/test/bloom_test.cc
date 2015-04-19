#include <iostream>
#include "common/logger.h"
#include "common/bloom_filter.h"

// g++ -std=c++11 -I  /home/amuralidharan/dev/hypocampd/src -o bloom_test bloom_test.cc ../bloom_filter.cc ../../murmurhash3.cc ../../logger.cc

using namespace hypocampd;

int main() {
    BloomFilter<> blm(10000000,0.001);
    blm.insert("Arun", 4);
    if (blm.can_have("Arun", 4)) {
        std::cout << "Key is present" << std::endl;
    }

    blm.insert("This is good!", 13);
    
    return 0;
}
