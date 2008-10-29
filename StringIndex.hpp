#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "Index.hpp"

namespace Ouzo
{
	class StringIndex : public Index
	{
		std::map< std::string, DocSet > m_map;
	public:
		typedef std::map< std::string, DocSet >::iterator iterator_type;
		typedef std::map< std::string, DocSet >::const_iterator const_iterator_type;
	
		StringIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: Index(index_file, keyspec, doccapacity) {}
	
		size_t keyCount() const { return m_map.size(); }
	
		// inline DocSet& operator[](const std::string& key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }
		inline const_iterator_type begin() const { return m_map.begin(); }
		inline const_iterator_type end() const { return m_map.end(); }
		inline iterator_type lower_bound(const char* key) { return m_map.lower_bound(key); }
		inline iterator_type lower_bound(const std::string& key) { return m_map.lower_bound(key); }
		inline const_iterator_type lower_bound(const std::string& key) const { return m_map.lower_bound(key); }

		void put(const char* key,docid_t docid);
		const DocSet& get(const char* key) const;
		void del(docid_t docid);
	
		void load();
		void save() const;

		friend ostream& operator<<(ostream& os, const StringIndex& idx);
		
	};

}

#endif
