#ifndef _OUZO_MUTEX_HPP
#define _OUZO_MUTEX_HPP

#include <string>

namespace Ouzo
{
	template<class T>
	class Mutex
	{
		T m_lock;
	public:
		Mutex(const std::string& s, bool exclusive=false)
			: m_lock(s.c_str())
		{
			if (exclusive)
				m_lock.lock();
			else
				m_lock.lock_sharable();
		}
		~Mutex()
		{
			m_lock.unlock();
		}
	
	};

}

#endif