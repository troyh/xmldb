#include <boost/interprocess/sync/file_lock.hpp>

#include "OuzoDB.hpp"

namespace Ouzo
{

void StringIndex::put(const char* key, docid_t docid)
{
	std::map<std::string,DocSet>::iterator itr=m_map.find(key);
	if (itr==m_map.end())
	{ // Doesn't yet exist in index
		DocSet docset(m_headerinfo.doccapacity);
		docset.set(docid);
		m_map.insert(make_pair(key,docset));
	}
	else
	{ // Update existing docset in index
		DocSet& docset(itr->second);
		docset.set(docid);
	}
	
}

const DocSet& StringIndex::get(const char* key) const
{
	std::map<std::string,DocSet>::const_iterator itr=m_map.find(key);
	return itr->second;
}

void StringIndex::del(docid_t docid)
{
	// Iterate the keys
	std::map<std::string,DocSet>::iterator itr_end=m_map.end();
	for(std::map<std::string,DocSet>::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		itr->second.clr(docid);
	}
}

void StringIndex::load()
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
	// Clear out m_map
	if (!m_map.empty())
		m_map.clear();
		
	std::ifstream ifs(m_filename.string().c_str());
	if (ifs.good())
	{
		readMeta(ifs);
	
		for(uint32_t i = 0; i < m_headerinfo.keycount; ++i)
		{
			// Read key
			std::string key;
			char buf[256];
			size_t len;
			ifs.read((char*)&len,sizeof(len));
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);
			
			ifs.read(buf,len);
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);

			buf[len]='\0';
			key=buf;
	
			// Read DocSet
			DocSet ds(m_headerinfo.doccapacity);
			ds.load(ifs);

			m_headerinfo.keysize=ds.sizeInBytes();

			// Put into index
			m_map.insert(make_pair(key,ds));
		}
	}
}

void StringIndex::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	// Write index meta info
	writeMeta(ofs);
	
	uint32_t cKeys=m_map.size();
	ofs.write((char*)&cKeys,sizeof(cKeys));
	
	std::map<std::string,DocSet>::const_iterator itr_end=m_map.end();
	for(std::map<std::string,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
	{
		// Write key
		std::string key=itr->first;
		size_t len=key.size();
		
		ofs.write((char*)&len,sizeof(len));
		ofs.write(key.c_str(),len);
		
		// Write DocSet
		itr->second.save(ofs);
	}
}

ostream& operator<<(ostream& os, const StringIndex& idx)
{
	std::map<std::string,DocSet>::const_iterator itr_end=idx.m_map.end();
	for(std::map<std::string,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
	{
		os << itr->first << ':' << endl;
	}
	return os;
}


}
