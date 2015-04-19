#include <iostream>
#include "common/reference_count.h"

using namespace hypocampd;

class Test : public AtomicRefCounter{
public:
    Test():ptr(new int[10]) {}
    ~Test() {
        std::cout << "Deleting" << std::endl;
        delete[] ptr;
    }
private:
    int* ptr;
};

using TestPtr = boost::intrusive_ptr<Test>;

int main() {
    TestPtr pp(new Test());
    {
        TestPtr pp1 = pp;
        std::cout << "Coming out of inner scope" << std::endl;
        std::cout << "Counter = " << pp1 -> get_count();
    }

    return 0;
}
