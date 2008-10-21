#ifndef _OUZO_STRINGINDEX_HPP
#define _OUZO_STRINGINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "DocSet.hpp"
#include "Index.hpp"

namespace Ouzo
{
	class StringIndex : public Index
	{
		std::map< std::string, DocSet > m_map;
	public:
		typedef std::map< std::string, DocSet >::iterator iterator_type;
	
		StringIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: Index(index_file, keyspec, doccapacity) {}
	
		size_t keyCount() const { return m_map.size(); }
	
		// inline DocSet& operator[](const std::string& key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }

		void put(const char* key,docid_t docid);
		const DocSet& get(const char* key) const;
		void del(docid_t docid);
	
		void load();
		void save() const;

		ostream& operator<<(ostream& os) const;
	};

}

#endif
