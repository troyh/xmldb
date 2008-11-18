#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "UIntIndex.hpp"
#include "Keys.hpp"

namespace Ouzo
{
	
	class StringIndex : public Index
	{
		// static Index::key_type STRING_TYPENAME;
	public:
		typedef std::map<std::string, Index::key_t> 				  lookup_table_type;
		typedef lookup_table_type::size_type			  size_type;

		typedef lookup_table_type::iterator 	  iterator;
		typedef lookup_table_type::const_iterator const_iterator;
		
	private:
		lookup_table_type m_lookup_table;
	public:
		
		class stringkey_t : public Index::key_t
		{
		public:
			stringkey_t(const Index* idx, const char* s="") : Index::key_t(idx) { m_val.ptr=new std::string(s); }
			stringkey_t(const Index* idx, const std::string& s) : Index::key_t(idx) { m_val.ptr=new std::string(s); }
			bool operator< (const Index::key_t& key) const;

			const std::string* string() const { return (std::string*)m_val.ptr; }
		};
	
		class StringIterator : public Index::Iterator
		{
			StringIndex::lookup_table_type::iterator m_itr;
			
		public:
			StringIterator(const StringIterator& itr) : Index::Iterator(itr), m_itr(itr.m_itr) {}
			StringIterator(StringIndex* idx, StringIndex::lookup_table_type::iterator itr) : Index::Iterator(idx), m_itr(itr) {}
			~StringIterator() {}
			
			StringIterator& operator=(const StringIterator& itr) { Index::Iterator::operator=(itr); m_itr=itr.m_itr; return *this; }
			
			bool operator==(const StringIterator& itr) { return m_itr==itr.m_itr; }
			bool operator!=(const StringIterator& itr) { return m_itr!=itr.m_itr; }
			
			StringIterator& operator++()    { m_itr++; return *this; }
			StringIterator& operator++(int) { m_itr++; return *this; }
			
			stringkey_t key() const  { return stringkey_t(m_idx,m_itr->first.c_str()); }
			DocSet& docset() { return m_idx->get(m_itr->second); }
		};
		
	
		StringIndex(const std::string& name, const key_type kt, const std::string& keyspec, uint32_t doccapacity) 
			: Index(name, kt, keyspec, doccapacity) {}
			
		key_t* createKey() const { return new stringkey_t(this); }
	
		Iterator* begin();
		Iterator* end();

		Iterator* lower_bound(const char* key);
		Iterator* lower_bound(const std::string& key);

		void put(const key_t& key,docid_t docid);
		
		      DocSet& get(const key_t& key);
		const DocSet& get(const key_t& key) const;
		
		void del(docid_t docid);

		// const DocSet& get(lookupid_t lookupid) const;
		// lookupid_t getLookupID(const char* val) const;
		
		void load();
		void save() const;

		void output(ostream&) const;
		
	};

}
#endif
