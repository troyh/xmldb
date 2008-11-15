#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "UIntIndex.hpp"

namespace Ouzo
{
	
	class StringIndex : public Index
	{
		static Index::key_type STRING_TYPENAME;
	public:
		typedef std::map<std::string, Key> 				  lookup_table_type;
		typedef lookup_table_type::size_type			  size_type;

		typedef lookup_table_type::iterator 	  iterator;
		typedef lookup_table_type::const_iterator const_iterator;
		
	private:
		lookup_table_type m_lookup_table;
	public:
	
		StringIndex(const std::string& name, const std::string& keyspec, uint32_t doccapacity) 
			: Index(name, STRING_TYPENAME, keyspec, doccapacity) {}
	
		inline Iterator       begin();// 									 { return m_lookup_table.begin(); }
		// inline const_iterator begin() const 							 { return m_lookup_table.begin(); }
		inline Iterator 	  end();// 									 { return m_lookup_table.end(); }
		// inline const_iterator end() const 								 { return m_lookup_table.end(); }

		inline Iterator 	  lower_bound(const char* key);// 			 	 { return m_lookup_table.lower_bound(key); }
		inline Iterator 	  lower_bound(const std::string& key);// 		 { return m_lookup_table.lower_bound(key); }
		// inline const_iterator lower_bound(const char* key) const 		 { return m_lookup_table.lower_bound(key); }
		// inline const_iterator lower_bound(const std::string& key) const  { return m_lookup_table.lower_bound(key); }

		void put(const Key& key,docid_t docid);
		
		      DocSet& get(const Key& key);
		const DocSet& get(const Key& key) const;
		
		void del(docid_t docid);

		// const DocSet& get(lookupid_t lookupid) const;
		// lookupid_t getLookupID(const char* val) const;
		
		void load();
		void save() const;

		void output(ostream&) const;
		
	};

}

#endif
