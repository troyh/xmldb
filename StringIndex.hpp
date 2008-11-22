#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "Index.hpp"
#include "Keys.hpp"

namespace Ouzo
{
	
	class StringIndex : public Index
	{
		// static Index::key_type STRING_TYPENAME;
	public:
		class stringkey_t : public Index::key_t
		{
			std::string m_s;
		public:
			stringkey_t(const char* s="") : Index::key_t(KEY_TYPE_STRING) { m_s=s; }
			stringkey_t(const std::string& s) : Index::key_t(KEY_TYPE_STRING) { m_s=s; }
			bool operator< (const Index::key_t& key) const;
			
			void assign(const char* s) { m_s=s; }

			const std::string& string() const { return m_s; }
			
			void output(ostream&) const;
			void outputBinary(ostream&) const;
			void inputBinary(istream&);
		};

		typedef std::map<stringkey_t, Index::key_t>		  lookup_table_type;
		typedef lookup_table_type::size_type			  size_type;

		typedef lookup_table_type::iterator 	  iterator;
		typedef lookup_table_type::const_iterator const_iterator;
		
	private:
		lookup_table_type m_lookup_table;
	public:
		static Index* createIndex(key_t::key_type kt, const char* name, const char* keyspec, uint32_t capacity);
		
	
		class StringIterator : public Index::Iterator
		{
			StringIndex::lookup_table_type::iterator m_itr;
			
		public:
			StringIterator(const StringIterator& itr) : Index::Iterator(itr), m_itr(itr.m_itr) {}
			StringIterator(StringIndex* idx, StringIndex::lookup_table_type::iterator itr) : Index::Iterator(idx), m_itr(itr) {}
			~StringIterator() {}
			
			StringIterator& operator=(const StringIterator& itr) { Index::Iterator::operator=(itr); m_itr=itr.m_itr; return *this; }
			
			bool operator==(const Iterator& itr) { return operator==((StringIterator&)itr); }
			bool operator!=(const Iterator& itr) { return operator!=((StringIterator&)itr); }
			bool operator==(const StringIterator& itr) { return m_itr==itr.m_itr; }
			bool operator!=(const StringIterator& itr) { return m_itr!=itr.m_itr; }
			
			StringIterator& operator++()    { m_itr++; return *this; }
			StringIterator& operator++(int) { m_itr++; return *this; }
			
			const Index::key_t& key() const  { return m_itr->first; }
			DocSet& docset() { return m_idx->get(m_itr->second); }
		};
		
	
		StringIndex(const std::string& name, const std::string& keyspec, uint32_t doccapacity) 
			: Index(name, Index::key_t::KEY_TYPE_STRING, keyspec, doccapacity) {}
			
		key_t::key_type baseKeyType() const;
			
			
		key_t* createKey() const { return new stringkey_t(); }
	
		Iterator* begin();
		Iterator* end();

		Iterator* lower_bound(const char* key);
		Iterator* lower_bound(const std::string& key);

		void put(const key_t& key,docid_t docid);
		
		      DocSet& get(const stringkey_t& key);
		const DocSet& get(const stringkey_t& key) const;
		
		bool del(docid_t docid, Index::key_t* pk=NULL);

		// const DocSet& get(lookupid_t lookupid) const;
		// lookupid_t getLookupID(const char* val) const;
		
		void load();
		void save() const;

		// void output(ostream&) const;
		
	};

}
#endif
