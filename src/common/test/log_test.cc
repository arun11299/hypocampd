#include "common/logger.h"
#include <utility>

/*
 g++ -std=c++11 -I /home/amuralidharan/dev/hypocampd/src -o log_test log_test.cc ../logger.cc
*/

using namespace hypocampd;

int main() {
   // LogRecorder<FileLogger> l;
    //l.log(0, "%d: %s", 54, "");
    FINFO("%s : %d", "Hello ! world.", 42);
    INFO("This is a test message");
   
    std::string s("This is a string message");
    INFO(std::move(s));
    return 0;
}
