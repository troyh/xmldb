#include <execinfo.h>
#include <stdlib.h>
#include <iostream>

#include "Exception.hpp"

namespace Ouzo
{
	Exception::Exception(const char* f,size_t ln) 
		: m_f(f), m_ln(ln), m_bt_strings(0), m_bt_size(0)
	{
		void *array[10];
		m_bt_size = backtrace (array, sizeof(array)/sizeof(array[0]));
		m_bt_strings = backtrace_symbols(array, m_bt_size);
	}

	Exception::~Exception() throw()
	{
		free(m_bt_strings);
	}

	std::ostream& operator<<(std::ostream& os, const Exception& x)
	{
		os << "Ouzo Exception: " << x.file() << '(' << x.line() << ')' << std::endl
			<< "Call stack:" << std::endl;
		for (size_t i = 0; i < x.m_bt_size; i++)
			os << i << ':' << x.m_bt_strings[i] << std::endl;
		return os;
	}
}
