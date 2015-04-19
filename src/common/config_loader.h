#ifndef HYPOCAMPD_CONFIG_LOADER_H
#define HYPOCAMPD_CONFIG_LOADER_H

#include <string>
#include <map>
#include <utility>
#include <fstream>
#include "common/logger.h"

namespace hypocampd {

    using PropertyMap = std::map<std::string, std::string>;

    class ConfigLoader {
    public:
	ConfigLoader(const std::string& file_name, const char* sep = "\t"):
			m_file_name(file_name), m_sep(sep) {}

	PropertyMap get_config() {

	    std::ifstream inf(m_file_name.c_str());
	    if (!inf) {
		FERROR("Unable to open server config file = %s", m_file_name.c_str());
		return PropertyMap();
	    }

	    std::string line;

	    while (std::getline(inf, line)) {
		if (line.find('#') == 0) continue;
		if (line == "") continue;

		size_t pos = line.find(m_sep);
		if (pos == std::string::npos) {
		    FERROR("Line is not tab delimited: %s", line.c_str());
		    continue;
		}

		m_props.insert(std::make_pair(line.substr(0, pos), 
					      line.substr(pos+1, line.size())));
		
	    } // end while

	    inf.close();
	    return m_props;
	}

	
	void print_map() const {
	    for (auto& kv : m_props) {
		FINFO("Key = %s ; Value = %s", kv.first.c_str(), kv.second.c_str());
	    }
	}

    private:
	std::string m_file_name;
	std::string m_sep;
	PropertyMap m_props;
    };

}; // END namespace hypocampd

#endif
