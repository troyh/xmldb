#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
extern "C"
{
#include <demangle.h> 
// demangle.h not installed in Debian with binutils-dev (it's a bug), I copied from
// http://www.koders.com/c/fid9CA7ECAFB6EF83304E94B7165B5205B3DEDF39DB.aspx?s=cdef%3Atree
}

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
		{
			os << i << '\t';
			
			char* p=strchr(x.m_bt_strings[i],'(');
			if (!p)
				os << x.m_bt_strings[i];
			else
			{
				++p;
				char* pp=strchr(p,')');
				if (!pp)
					os << p;
				else
				{
					*pp='\0';
					char* res=cplus_demangle(p, DMGL_ANSI /*| DMGL_PARAMS*/); // TODO: make DMGL_PARAMS work
					if (!res)
						os << p;
					else
					{
						os << res;
						free(res);
					}
				}
			}
			os << std::endl;
		}
			
			
		return os;
	}
}
