#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

void StringIndex::put(const char* key, docid_t docid)
{
	lookupid_t lookupid=getLookupID(key);
	if (lookupid==0)
	{ // Doesn't yet exist in index
	
		// Add it to the m_lookup_table by creating a lookupid
		// TODO: lock m_lookup_table
		lookupid=m_lookup_table.size();
		m_lookup_table.insert(make_pair(key,lookupid));
		// TODO: unlock m_lookup_table
	
		DocSet docset(m_headerinfo.doccapacity);
		docset.set(docid);
		
		m_map.insert(make_pair(lookupid,docset));
		m_headerinfo.doccount++;
		m_headerinfo.keycount++;
	}
	else
	{ // Update existing docset in index
		std::map<lookupid_t,DocSet> itr=m_map.find(lookupid);
		if (itr==m_map.end())
			throw Exception(__FILE__,__LINE__);
			
		DocSet& docset(itr->second);
		docset.set(docid);
	}
	
}

const DocSet& StringIndex::get(const char* key) const
{
	lookupid_t lookupid=getLookupID(key);
	
	std::map<lookupid_t,DocSet>::const_iterator itr=m_map.find(lookupid);
	if (itr==m_map.end())
		throw Exception(__FILE__,__LINE__);
		
	return itr->second;
}

Index::lookupid_t StringIndex::getLookupID(const char* val) const
{
	std::map<std::string,lookupid_t>::const_iterator itr=m_lookup_table.find(key);
	if (itr==m_lookup_table.end())
		return 0;
	return itr->second;
}

void StringIndex::del(docid_t docid)
{
	// Iterate the keys
	std::map<lookupid_t,DocSet>::iterator itr_end=m_map.end();
	for(std::map<lookupid_t,DocSet>::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		if (itr->second.test(docid)) // If the bit is set
		{
			itr->second.clr(docid);
			
			// Remove it from m_lookup_table
			for (std::map<std::string,lookupid_t> itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
			{
				if (itr2->second==itr->first)
					m_lookup_table.clear(itr2->first);
			}
		}
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
		
		if (ifs.good())
		{
			if (m_headerinfo.type==INDEX_TYPE_UNKNOWN && m_headerinfo.keycount==0)
				m_headerinfo.type=INDEX_TYPE_STRING;
			else if (m_headerinfo.type!=INDEX_TYPE_STRING)
				throw Exception(__FILE__,__LINE__);

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
	
}

void StringIndex::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	// Write index meta info
	writeMeta(ofs);
	
	uint32_t cKeys=m_map.size();
	
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
	os << (Index&)idx << endl;
	StringIndex& idx2=(StringIndex&)(idx); // cast away const-ness because C++ is kinda dumb this way
	std::map<std::string,DocSet>::const_iterator itr_end=idx.m_map.end();
	for(std::map<std::string,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
	{
		const DocSet& ds=idx2.get(itr->first.c_str());
		os << itr->first << ':' << ds << endl;
	}
	return os;
}


}
