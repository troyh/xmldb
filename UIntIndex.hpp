#ifndef _OUZO_UINTINDEX_HPP
#define _OUZO_UINTINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "Index.hpp"

namespace Ouzo
{

	class UIntIndex : public Index
	{
		std::map< uint32_t, DocSet > m_map;
	public:
		typedef std::map< uint32_t, DocSet >::iterator iterator_type;

		UIntIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) : Index(index_file, keyspec, doccapacity) {}

		size_t keyCount() const { return m_map.size(); }

		// inline DocSet& operator[](uint32_t key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }

		void put(const char* key,docid_t docid);
		const DocSet& get(const char* key) const { return get(strtoul(key,0,10)); }
		const DocSet& get(uint32_t key) const;
		void del(docid_t docid);

		void load();
		void save() const;
	
		friend ostream& operator<<(ostream&,const UIntIndex&);
	};

	ostream& operator<<(ostream& os, const UIntIndex& idx);

}

#endif
