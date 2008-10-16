#include <boost/interprocess/sync/file_lock.hpp>

#include "OuzoDB.hpp"

namespace Ouzo
{

void UIntIndex::put(const char* key, docid_t docid)
{
	uint32_t val=strtoul(key,0,10);
	
	std::map<uint32_t,DocSet>::iterator itr=m_map.find(val);
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

const DocSet& UIntIndex::get(uint32_t key) const
{
	std::map<uint32_t,DocSet>::const_iterator itr=m_map.find(key);
	return itr->second;
}

void UIntIndex::del(docid_t docid)
{
	// Iterate the keys
	std::map<uint32_t,DocSet>::iterator itr_end=m_map.end();
	for(std::map<uint32_t,DocSet>::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		itr->second.clr(docid);
	}
}

void UIntIndex::load()
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
	// Clear out m_map
	if (!m_map.empty())
		m_map.clear();

	std::ifstream ifs(m_filename.string().c_str());
	if (ifs.good())
	{
		readMeta(ifs);
	
		for (uint32_t i=0;i<m_headerinfo.keycount;++i)
		{
			// Read key
			uint32_t n;
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

void UIntIndex::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	if (!ofs.good())
		throw Exception(__FILE__,__LINE__);
	
	// Write index meta info
	writeMeta(ofs);

	std::map<uint32_t,DocSet>::const_iterator itr_end=m_map.end();
	for(std::map<uint32_t,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
	{
		// Write key
		uint32_t n=itr->first;
		ofs.write((char*)&n,sizeof(n));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
		
		// Write DocSet
		itr->second.save(ofs);
	}
}

ostream& operator<<(ostream& os, const UIntIndex& idx)
{
	UIntIndex& idx2=(UIntIndex&)idx; // cast away const-ness because C++ is kinda dumb this way
	std::map<uint32_t,DocSet>::const_iterator itr_end=idx.m_map.end();
	for(std::map<uint32_t,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
	{
		const DocSet& ds=idx2.get(itr->first);
		os << itr->first << ':' << ds << endl;
	}
	return os;
}


}
