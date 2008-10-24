#ifndef _OUZO_EXCEPTION_HPP
#define _OUZO_EXCEPTION_HPP

#include <iostream>

namespace Ouzo
{
	class Exception : public std::exception
	{
		const char* const m_f;
		size_t m_ln;
		char** m_bt_strings;
		size_t m_bt_size;
	public:
		Exception(const char* f,size_t ln);
		virtual ~Exception() throw();
	
		const char* const file() const { return m_f; }
		size_t line() const { return m_ln; }
		
		friend std::ostream& operator<<(std::ostream& os, const Exception& x);
	};
	
	std::ostream& operator<<(std::ostream& os, const Exception& x);

}

#endif
