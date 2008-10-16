#ifndef _OUZO_EXCEPTION_HPP
#define _OUZO_EXCEPTION_HPP

namespace Ouzo
{
	class Exception : public std::exception
	{
		const char* const m_f;
		size_t m_ln;
	public:
		Exception(const char* f,size_t ln) : m_f(f), m_ln(ln) {}
		virtual ~Exception() throw() {}
	
		const char* const file() const { return m_f; }
		size_t line() const { return m_ln; }
	};

}

#endif
