#include <iostream>
#include <string>
#include "common/aligned_storage.h"

using namespace hypocampd;


struct SC {
    char c;    
};

int main() {
    uint32_t val = 42;
    AlStore<uint32_t> al(val);

    std::cout << al.object() << std::endl;

    std::string s("Test string");
    std::cout << std::is_pod<std::string>::value << std::endl;
    AlStore<std::string> al2(s);

    std::cout << al2.object() << std::endl;

    SC c; c.c = 'a';

    AlStore<SC> al3(c);
    std::cout << alignof(al3) << std::endl;

    AlStore<uint32_t> tmp = std::move(AlStore<uint32_t>(69));
    std::cout << tmp.object() << std::endl;

    return 0;
}
