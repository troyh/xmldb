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

}
