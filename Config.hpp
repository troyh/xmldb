#ifndef _OUZO_CONFIG_HPP
#define _OUZO_CONFIG_HPP

#include <iostream>
#include <string>

namespace Ouzo
{
	using namespace std;
	
	class Config
	{
		friend ostream& operator<<(ostream& os, const Config& cfg);
		
		std::map<std::string,std::string> m_info;
	public:
		Config() {}
		~Config() {}
	
		inline std::string get(const std::string& s) const { return ((Config*)this)->m_info[s]; }
		void set(std::string name, std::string value);
		void set(std::string name, uint32_t value);
	};

	ostream& operator<<(ostream& os, const Config& cfg);
	
}

#endif
