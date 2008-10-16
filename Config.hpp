#ifndef _OUZO_CONFIG_HPP
#define _OUZO_CONFIG_HPP

#include <string>

namespace Ouzo
{

	class Config
	{
		std::map<std::string,std::string> m_info;
	public:
		Config() {}
		~Config() {}
	
		inline std::string get(const std::string& s) const { return ((Config*)this)->m_info[s]; }
		void set(std::string name, std::string value);
		void set(std::string name, uint32_t value);
	};

}

#endif
