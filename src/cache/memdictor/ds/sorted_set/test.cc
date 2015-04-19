#include "skip_list.h"
#include <iostream>
#include <string>

using namespace hypocampd;

bool test_skip_node() {
    std ::cout << "Testing skip node" << std::endl;
    std::string* str = new std::string("Arun");
    int* val =  new int(10);
    SkipListNode<std::string, int> node(str, val, 4);
    std::cout << *node.get_key() << std::endl;
    return true;
}

void test_random() {
    HeightHelper h(4, 0.5);
    for (int i = 0; i < 20 ; i++)
        std::cout << (int)h.new_height() << std::endl;
}

void test_iterator(SkipList<std::string, int>& list) ;

bool test_skip_list() {
    std::cout << "Testing skip list" << std::endl;
    std::string* max_key = new std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");
    SkipList<std::string, int> list(4, 0.5, max_key);

    std::string* str = new std::string("try");
    list.insert(str, new int(10));

    std::string* str1 = new std::string("try1");
    list.insert(str1, new int(10));

    std::string* str2 = new std::string("try2");
    list.insert(str2, new int(10));

    std::string* str3 = new std::string("try3");
    list.insert(str3, new int(10));

    std::string* str4 = new std::string("try4");
    list.insert(str4, new int(10));

    std::string* str5 = new std::string("try5");
    list.insert(str5, new int(10));

    std::string* str6 = new std::string("try6");
    list.insert(str6, new int(10));

    std::string* str7 = new std::string("try7");
    list.insert(str7, new int(10));

    std::string* str8 = new std::string("try8");
    list.insert(str8, new int(10));

    std::string* str9 = new std::string("try9");
    list.insert(str9, new int(10));

    std::string* str10 = new std::string("try10");
    list.insert(str10, new int(10));

    std::string* str11 = new std::string("try11");
    list.insert(str11, new int(10));

    std::string* str12 = new std::string("try12");
    list.insert(str12, new int(10));

    std::cout << "Finding: " << list.find(str) << std::endl;

    list.remove(str);

    std::cout << "Finding: " << list.find(str) << std::endl;
    list.remove(str1);
    list.__dump__();


    test_iterator(list);
}

void test_iterator(SkipList<std::string, int>& list) {

    std::cout << "Iterator Testing" << std::endl;

    SkipList<std::string, int>::iterator it = list.begin();

    for (; it != list.end(); ++it) {
        std::cout << *((*it).get_key()) << std::endl;
    }
}

int main() {
    test_skip_node();
    test_skip_list();
    //test_random();
    return 0;
}
