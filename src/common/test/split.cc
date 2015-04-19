#include <iostream>
#include <string>
#include <map>
#include <boost/tokenizer.hpp>
#include <iterator>

using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

class file {
public:
    static const char arg[];
};
decltype(file::arg) file::arg = ":";


template <const char* ptr>
class Token {
public:
    void parse(const std::string& s) {
	boost::char_separator<char> sep(ptr);	
	Tokenizer tokens(s, sep);
    }
};

int main() {
    Token<file::arg> tt;
    tt.parse("This:is:a:tokenized:string");
    return 0;
}
