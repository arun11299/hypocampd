#include <iostream>
#include "../simple_string.h"

using namespace hypocampd;

void test_assign() {
    std::cout << "---------------------------" << std::endl;
    std::cout << "Testing assignment" << std::endl;

    sstring s1("String1");
    sstring s2;
    s2 = s1;
    s2.__dump__();

    sstring s3;
    s3 = s2;
    std::cout << "s3 = " << s3.c_str() << std::endl;
    s3.append("Should move away from SSO");
    std::cout << s3.c_str() << std::endl;
    s3.__dump__();

    s3 = s1;
    std::cout << s3.c_str() << std::endl;
    s3.__dump__();


    sstring s4(std::move(s3));
    std::cout << "After move ctor = " << s4.c_str() << std::endl;
    s4.__dump__();

    std::cout << "---END assignment test---" << "\n";
    std::cout << std::endl;
}

void test_substr() {
    std::cout << "Testing substring" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    sstring s1("This is my string");
    std::cout << s1.c_str() << std::endl;

    s1.append(" string complete. This is going to test it");
    std::cout << "After Append = " << s1.c_str() << std::endl;
    
    s1.__dump__();

    s1.shrink_to_fit();
    std::cout << "After shrink = " << s1.c_str() << std::endl;
    s1.__dump__();

    std::cout << "----END substr test----------" << "\n";
    std::cout << std::endl;
}

int main() {
    test_assign();
    test_substr();

    std::cout << sizeof(sstring) << std::endl;

    return 0;
}
