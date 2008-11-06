#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "UIntIndex.hpp"

namespace Ouzo
{
	class StringIndex : public UIntIndex<uint32_t>
	{
	public:
		typedef std::map<std::string, lookupid_t> 				  lookup_table_type;

		typedef std::map<std::string, lookupid_t>::iterator 	  iterator_type;
		typedef std::map<std::string, lookupid_t>::const_iterator const_iterator_type;
		
	private:
		lookup_table_type m_lookup_table;
	public:
	
		StringIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<uint32_t>(name, index_file, keyspec, doccapacity) {}
	
		inline iterator_type       begin() 									 { return m_lookup_table.begin(); }
		inline iterator_type 	   end() 									 { return m_lookup_table.end(); }
		inline const_iterator_type begin() const 							 { return m_lookup_table.begin(); }
		inline const_iterator_type end() const 								 { return m_lookup_table.end(); }
		inline iterator_type 	   lower_bound(const char* key) 			 { return m_lookup_table.lower_bound(key); }
		inline iterator_type 	   lower_bound(const std::string& key) 		 { return m_lookup_table.lower_bound(key); }
		inline const_iterator_type lower_bound(const std::string& key) const { return m_lookup_table.lower_bound(key); }

		void put(const char* key,docid_t docid);
		void del(docid_t docid);
		
		const DocSet& get(const char* key) const;
		const DocSet& get(lookupid_t lookupid) const;
		
		lookupid_t getLookupID(const char* val) const;
		
		void load();
		void save() const;
	
		friend ostream& operator<<(ostream& os, const StringIndex& idx);
		
	};

}

#endif
