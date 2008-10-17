#include <sstream>

#include "Ouzo.hpp"

namespace Ouzo
{
	
void Config::set(std::string name, std::string value)
{
	m_info[name]=value;
}

void Config::set(std::string name, uint32_t value)
{
	std::ostringstream n;
	n << value;
	m_info[name]=n.str();
}

ostream& operator<<(ostream& os, const Config& cfg)
{
	std::map<std::string,std::string>::const_iterator itr_end=cfg.m_info.end();
	for(std::map<std::string,std::string>::const_iterator itr=cfg.m_info.begin(); itr != itr_end; ++itr)
	{
		os << itr->first << '=' << itr->second << endl;
	}
	
	return os;
}

}
