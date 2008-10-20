#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

// template<typename UINT_TYPE>
// void UIntIndex<UINT_TYPE>::put(const char* key, docid_t docid)
// {
// 	UINT_TYPE val=strtoul(key,0,10);
// 	
// 	iterator_type itr=m_map.find(val);
// 	if (itr==m_map.end())
// 	{ // Doesn't yet exist in index
// 		DocSet docset(m_headerinfo.doccapacity);
// 		docset.set(docid);
// 		
// 		m_map.insert(make_pair(val,docset));
// 		m_headerinfo.doccount++;
// 		m_headerinfo.keycount++;
// 	}
// 	else
// 	{ // Update existing docset in index
// 		DocSet& docset=itr->second;
// 		docset.set(docid);
// 	}
// }

// template<typename UINT_TYPE>
// const DocSet& UIntIndex<UINT_TYPE>::get(UINT_TYPE key) const
// {
// 	const_iterator_type itr=m_map.find(key);
// 	return itr->second;
// }

// template<typename UINT_TYPE>
// void UIntIndex<UINT_TYPE>::del(docid_t docid)
// {
// 	// Iterate the keys
// 	iterator_type itr_end=m_map.end();
// 	for(iterator_type itr=m_map.begin(); itr!=itr_end; ++itr)
// 	{
// 		// Remove the docid from the DocSet
// 		itr->second.clr(docid);
// 	}
// }

// template<typename UINT_TYPE>
// void UIntIndex<UINT_TYPE>::load()
// {
// 	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
// 	
// 	// Clear out m_map
// 	if (!m_map.empty())
// 		m_map.clear();
// 
// 	std::ifstream ifs(m_filename.string().c_str());
// 	if (ifs.good())
// 	{
// 		readMeta(ifs);
// 		
// 		if (m_headerinfo.type!=INDEX_TYPE_UINT32)
// 			throw Exception(__FILE__,__LINE__);
// 	
// 		for (uint32_t i=0;i<m_headerinfo.keycount;++i)
// 		{
// 			// Read key
// 			UINT_TYPE n;
// 			ifs.read((char*)&n,sizeof(n));
// 
// 			if (!ifs.good())
// 				throw Exception(__FILE__,__LINE__);
// 
// 			// Read DocSet
// 			DocSet ds(m_headerinfo.doccapacity);
// 			ds.load(ifs);
// 			
// 			m_headerinfo.keysize=ds.sizeInBytes();
// 
// 			// Put into index
// 			m_map.insert(make_pair(n,ds));
// 		}
// 	}
// }

// template<typename UINT_TYPE>
// void UIntIndex<UINT_TYPE>::save() const
// {
// 	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);
// 
// 	std::ofstream ofs(m_filename.string().c_str());
// 	if (!ofs.good())
// 		throw Exception(__FILE__,__LINE__);
// 	
// 	// Write index meta info
// 	writeMeta(ofs);
// 
// 	const_iterator_type itr_end=m_map.end();
// 	for(const_iterator_type itr = m_map.begin(); itr != itr_end; ++itr)
// 	{
// 		// Write key
// 		UINT_TYPE n=itr->first;
// 		ofs.write((char*)&n,sizeof(n));
// 		if (!ofs.good())
// 			throw Exception(__FILE__,__LINE__);
// 		
// 		// Write DocSet
// 		itr->second.save(ofs);
// 	}
// }

// template<typename UINT_TYPE>
// ostream& operator<<(ostream& os, const UIntIndex<UINT_TYPE>& idx)
// {
// 	UIntIndex<UINT_TYPE>& idx2=(UIntIndex<UINT_TYPE>&)idx; // cast away const-ness because C++ is kinda dumb this way
// 	// UIntIndex<UINT_TYPE>::const_iterator_type itr_end=idx.m_map.end();
// 	// for(UIntIndex<UINT_TYPE>::const_iterator_type itr = idx.m_map.begin(); itr != itr_end; ++itr)
// 	// {
// 	// 	const DocSet& ds=idx2.get(itr->first);
// 	// 	os << itr->first << ':' << ds << endl;
// 	// }
// 	return os;
// }


}
