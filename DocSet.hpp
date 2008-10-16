#ifndef _OUZO_DOCSET_HPP
#define _OUZO_DOCSET_HPP

#include <iostream>
#include <map>
#include <vector>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>

#include "Ouzo.hpp"
#include "BitmapAllocator.hpp"

namespace Ouzo
{
	using namespace std;
	using namespace boost;
	
	class DocSet
	{
	public:
		typedef enum { arr, bitmap } set_type;
		typedef dynamic_bitset< unsigned long,BitmapAllocator<unsigned long> > bitset_type;
	private:
		// union DocUnion
		// {
			boost::shared_ptr< std::vector<docid_t> > m_docs_arr;
			boost::shared_ptr<bitset_type> m_docs_bitmap;
		// } m_docs;
		set_type m_type;
		size_t m_capacity;
	
		set_type mostEfficientType() const;
		void convertToType(set_type t);
	
	public:
		DocSet(size_t capacity);
		DocSet(DocSet& ds);
		DocSet(const DocSet& ds);
		~DocSet() {}
	
		DocSet& operator=(DocSet& ds);
		DocSet& operator=(const DocSet& ds);
	
		set_type type() const { return m_type; }
		size_t size() const;
		uint32_t sizeInBytes() const;
	
		void set(docid_t docno);
		void clr(docid_t docno);
	
		DocSet& operator|=(const DocSet& ds);
		DocSet& operator&=(const DocSet& ds);
	
		void load(istream& is);

		void save(ostream& os) const;
	
		friend ostream& operator<<(ostream& os, const DocSet& ds);
	
	};

	ostream& operator<<(ostream& os, const DocSet& ds);
}

#endif
