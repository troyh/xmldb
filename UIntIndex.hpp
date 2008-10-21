#ifndef _OUZO_UINTINDEX_HPP
#define _OUZO_UINTINDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include "Index.hpp"

namespace Ouzo
{

	template<typename UINT_TYPE>
	class UIntIndex;
	
	template<typename UINT_TYPE>
	ostream& operator<<(ostream& os, const UIntIndex<UINT_TYPE>& idx);

	template<typename UINT_TYPE=uint32_t>
	class UIntIndex : public Index
	{
		typedef std::map< UINT_TYPE, DocSet > map_type;

		map_type m_map;
	public:

		typedef typename map_type::iterator iterator_type;
		typedef typename map_type::const_iterator const_iterator_type;

		UIntIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: Index(index_file, keyspec, doccapacity) {}

		size_t keyCount() const { return m_map.size(); }

		// inline DocSet& operator[](UINT_TYPE key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }
		
		void put(const char* key,docid_t docid)
		{
			UINT_TYPE val=strtoul(key,0,10);

			iterator_type itr=m_map.find(val);
			if (itr==m_map.end())
			{ // Doesn't yet exist in index
				DocSet docset(m_headerinfo.doccapacity);
				docset.set(docid);

				m_map.insert(make_pair(val,docset));
				m_headerinfo.doccount++;
				m_headerinfo.keycount++;
			}
			else
			{ // Update existing docset in index
				DocSet& docset=itr->second;
				docset.set(docid);
			}
		}
		
		const DocSet& get(const char* key) const { return get(strtoul(key,0,10)); }
		const DocSet& get(UINT_TYPE key) const
		{
			const_iterator_type itr=m_map.find(key);
			return itr->second;
		}

		void del(docid_t docid)
		{
			// Iterate the keys
			iterator_type itr_end=m_map.end();
			for(iterator_type itr=m_map.begin(); itr!=itr_end; ++itr)
			{
				// Remove the docid from the DocSet
				itr->second.clr(docid);
			}
		}

		void load()
		{
			Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);

			// Clear out m_map
			if (!m_map.empty())
				m_map.clear();

			std::ifstream ifs(m_filename.string().c_str());
			if (ifs.good())
			{
				readMeta(ifs);

				if (m_headerinfo.type!=INDEX_TYPE_UINT32)
					throw Exception(__FILE__,__LINE__);

				for (uint32_t i=0;i<m_headerinfo.keycount;++i)
				{
					// Read key
					UINT_TYPE n;
					ifs.read((char*)&n,sizeof(n));

					if (!ifs.good())
						throw Exception(__FILE__,__LINE__);

					// Read DocSet
					DocSet ds(m_headerinfo.doccapacity);
					ds.load(ifs);

					m_headerinfo.keysize=ds.sizeInBytes();

					// Put into index
					m_map.insert(make_pair(n,ds));
				}
			}
		}
		
		void save() const
		{
			Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

			std::ofstream ofs(m_filename.string().c_str());
			if (!ofs.good())
				throw Exception(__FILE__,__LINE__);

			// Write index meta info
			writeMeta(ofs);

			const_iterator_type itr_end=m_map.end();
			for(const_iterator_type itr = m_map.begin(); itr != itr_end; ++itr)
			{
				// Write key
				UINT_TYPE n=itr->first;
				ofs.write((char*)&n,sizeof(n));
				if (!ofs.good())
					throw Exception(__FILE__,__LINE__);

				// Write DocSet
				itr->second.save(ofs);
			}
		}
	
		friend ostream& operator<< <UINT_TYPE> (ostream& os, const UIntIndex<UINT_TYPE>& idx);
	};

	template<typename UINT_TYPE>
	ostream& operator<<(ostream& os, const UIntIndex<UINT_TYPE>& idx) 
	{
		os << "UIntIndex::operator<<()" << endl;
		UIntIndex<UINT_TYPE>& idx2=(UIntIndex<UINT_TYPE>&)(idx); // cast away const-ness because C++ is kinda dumb this way
		typename UIntIndex<UINT_TYPE>::const_iterator_type itr_end=idx.m_map.end();
		for(typename UIntIndex<UINT_TYPE>::const_iterator_type itr = idx.m_map.begin(); itr != itr_end; ++itr)
		{
			const DocSet& ds=idx2.get(itr->first);
			os << itr->first << ':' << ds << endl;
		}
		return os;
		
	}

	template<typename INT_TYPE>
	class IntIndex : public UIntIndex<INT_TYPE>
	{
	public:
		IntIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<INT_TYPE>(index_file, keyspec, doccapacity) {}
	};
	
	class DateIndex : public UIntIndex<uint32_t>
	{
	public:
		DateIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<uint32_t>(index_file, keyspec, doccapacity) {}
	};
	
	class TimeIndex : public UIntIndex<time_t>
	{
	public:
		TimeIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<time_t>(index_file, keyspec, doccapacity) {}
	};
	
	class FloatIndex : public UIntIndex<double>
	{
	public:
		FloatIndex(const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<double>(index_file, keyspec, doccapacity) {}
	};

}

#endif
