#ifndef _OUZO_UINTINDEX_HPP
#define _OUZO_UINTINDEX_HPP

/*
#include <iostream>
#include <fstream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include "Index.hpp"
#include "Mutex.hpp"
#include "Exception.hpp"

namespace Ouzo
{

	template<typename UINT_TYPE>
	class UIntIndex;
	
	template<typename UINT_TYPE>
	ostream& operator<<(ostream& os, const UIntIndex<UINT_TYPE>& idx);

	template<typename UINT_TYPE=uint32_t>
	class UIntIndex : public Index
	{
	public:
		typedef std::map< UINT_TYPE, DocSet > map_type;
	protected:
		map_type m_map;
	public:

		typedef typename map_type::iterator iterator_type;
		typedef typename map_type::const_iterator const_iterator_type;

		UIntIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: Index(name, index_file, keyspec, doccapacity) {}

		size_t keyCount() const { return m_map.size(); }

		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }
		inline const_iterator_type begin() const { return m_map.begin(); }
		inline const_iterator_type end() const { return m_map.end(); }

		inline iterator_type       lower_bound(const std::string& key) { return m_map.lower_bound(strtoul(key.c_str(),0,10)); }
		inline iterator_type       lower_bound(UINT_TYPE key) { return m_map.lower_bound(key); }
		inline const_iterator_type lower_bound(const std::string& key) const { return m_map.lower_bound(strtoul(key.c_str(),0,10)); }
		inline const_iterator_type lower_bound(UINT_TYPE key) const { return m_map.lower_bound(key); }
		
		void put(UINT_TYPE key,docid_t docid)
		{
			iterator_type itr=m_map.find(key);
			if (itr==m_map.end())
			{ // Doesn't yet exist in index
				DocSet docset(m_headerinfo.doccapacity);
				docset.set(docid);

				m_map.insert(make_pair(key,docset));
				m_headerinfo.doccount++;
				m_headerinfo.keycount++;
			}
			else
			{ // Update existing docset in index
				DocSet& docset=itr->second;
				docset.set(docid);
			}
		}
		
		void put(const char* key,docid_t docid)
		{
			UINT_TYPE val=strtoul(key,0,10);
			put(val,docid);
		}
		
		void put(const std::string& key,docid_t docid)
		{ 
			put(key.c_str(), docid); 
		}
		
		
		const DocSet& get(const char* key) const { return get(strtoul(key,0,10)); }
		const DocSet& get(UINT_TYPE key) const
		{
			const_iterator_type itr=m_map.find(key);
			return itr->second;
		}
		const UINT_TYPE getKeyForLookupID(Index::lookupid_t lookupid) const
		{
			return static_cast<UINT_TYPE>(lookupid);
		}
		lookupid_t getLookupID(const char* val) const
		{
			UINT_TYPE n=strtoul(val,0,10);
			return n;
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
			
			std::ifstream ifs(m_filename.string().c_str());
			
			load_data(ifs);
		}
		
		void load_data(istream& ifs)
		{
			// Clear out m_map
			if (!m_map.empty())
				m_map.clear();

			if (ifs.good())
			{
				readMeta(ifs);
				
				if (ifs.good())
				{
					if (m_headerinfo.type==INDEX_TYPE_UNKNOWN && m_headerinfo.keycount==0)
						m_headerinfo.type=INDEX_TYPE_UINT32;
					// Note: we can't expect INDEX_TYPE_UINT32, because this is a base for many similar types
					// else if (m_headerinfo.type!=INDEX_TYPE_UINT32)
					// 	throw Exception(__FILE__,__LINE__);

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
		}
		
		void save_data(ostream& ofs) const
		{
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
		
		void save() const
		{
			Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

			std::ofstream ofs(m_filename.string().c_str());
			
			save_data(ofs);
		}
	
		friend ostream& operator<< <UINT_TYPE> (ostream& os, const UIntIndex<UINT_TYPE>& idx);
	};

	template<typename UINT_TYPE>
	ostream& operator<<(ostream& os, const UIntIndex<UINT_TYPE>& idx) 
	{
		os << (Index&)(idx) << endl;
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
		IntIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<INT_TYPE>(name, index_file, keyspec, doccapacity) {}
	};
	
	class DateIndex : public UIntIndex<uint32_t>
	{
	public:
		DateIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<uint32_t>(name, index_file, keyspec, doccapacity) {}
	};
	
	class TimeIndex : public UIntIndex<time_t>
	{
	public:
		TimeIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<time_t>(name, index_file, keyspec, doccapacity) {}
	};
	
	class FloatIndex : public UIntIndex<double>
	{
	public:
		FloatIndex(const std::string& name, const bfs::path& index_file, const std::string& keyspec, uint32_t doccapacity) 
			: UIntIndex<double>(name, index_file, keyspec, doccapacity) {}
	};

}
*/

#endif
