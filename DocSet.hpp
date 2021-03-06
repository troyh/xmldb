#ifndef _OUZO_DOCSET_HPP
#define _OUZO_DOCSET_HPP

#include <iostream>
#include <map>
#include <vector>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>

#include "BitmapAllocator.hpp"

namespace Ouzo
{
	using namespace std;
	using namespace boost;

	typedef uint32_t docid_t;
	
	class DocSet
	{
	public:
		typedef enum { /*arr,*/ bitmap=1, docid } set_type;
		typedef dynamic_bitset< unsigned long,BitmapAllocator<unsigned long> > bitset_type;
		typedef bitset_type::size_type size_type;
		static const size_type npos=bitset_type::npos;
	private:
		// union DocUnion
		// {
			// boost::shared_ptr< std::vector<docid_t> > m_docs_arr;
			boost::shared_ptr<bitset_type> m_docs_bitmap;
			docid_t m_docs_docid;
		// } m_docs;
		set_type m_type;
		size_t m_capacity;
	
		set_type mostEfficientType() const;
		void convertToType(set_type t);
	
	public:
		DocSet(size_t capacity);
		DocSet(DocSet& ds);
		DocSet(const DocSet& ds);
		~DocSet() { m_docs_bitmap.reset((bitset_type*)0); }
	
		DocSet& operator=(DocSet& ds);
		DocSet& operator=(const DocSet& ds);
	
		set_type type() const { return m_type; }
		size_type size() const;
		uint32_t sizeInBytes() const;
		size_type count() const { return m_docs_bitmap->count(); }
		
		bool isNil() const { return count()==0; }
		
		bool test(docid_t docno) const;
	
		void set(docid_t docno);
		void clr(docid_t docno);
		
		void flip();
	
		DocSet& operator|=(const DocSet& ds);
		DocSet& operator&=(const DocSet& ds);
	
		void load(istream& is);

		void save(ostream& os) const;
		
		size_type find_first() const;
		size_type find_next(size_type n) const;
	
		friend ostream& operator<<(ostream& os, const DocSet& ds);
	
	};

	ostream& operator<<(ostream& os, const DocSet& ds);
}

#endif
