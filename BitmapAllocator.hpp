#ifndef _OUZO_BITMAPALLOCATOR_HPP
#define _OUZO_BITMAPALLOCATOR_HPP

namespace Ouzo
{
	template <class T>
	class BitmapAllocator
	{
	public:
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;
	private:
		pointer m_ptr;
		size_type m_size;
	public:

		BitmapAllocator() {}
		BitmapAllocator(const BitmapAllocator& ba)
		{
			m_ptr=ba.m_ptr;
			m_size=ba.m_size;
		}

		pointer   allocate(size_type n, const void * = 0) 
		{
			T* t = (T*) malloc(n * sizeof(T));
			m_ptr=t;
			m_size=n*sizeof(T);
			return t;
		}

		void      deallocate(void* p, size_type) 
		{
			if (p)
			{
				free(p);
			} 
		}

		pointer           address(reference x) const { return &x; }
		const_pointer     address(const_reference x) const { return &x; }
		BitmapAllocator<T>&  operator=(const BitmapAllocator&) { return *this; }
		void              construct(pointer p, const T& val) { new ((T*) p) T(val); }
		void              destroy(pointer p) { p->~T(); }

		size_type         max_size() const { return size_t(-1); }

		template <class U>
		struct rebind { typedef BitmapAllocator<U> other; };

		template <class U>
		BitmapAllocator(const BitmapAllocator<U>&) {
		}

		template <class U>
		BitmapAllocator& operator=(const BitmapAllocator<U>& ba) { 
			m_ptr=ba.m_ptr;
			m_size=ba.m_size;
			return *this; 
		}

		size_type sizeInBytes() const { return m_size; }
		char* startOfSpace() { return (char*)m_ptr; }
	};

}

#endif
