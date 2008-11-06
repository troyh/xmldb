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
		map_type::iterator itr=m_map.find(lookupid);
		if (itr==m_map.end())
			throw Exception(__FILE__,__LINE__);
			
		DocSet& docset(itr->second);
		docset.set(docid);
	}
	
}

const DocSet& StringIndex::get(const char* key) const
{
	lookupid_t lookupid=getLookupID(key);
	
	map_type::const_iterator itr=m_map.find(lookupid);
	if (itr==m_map.end())
		throw Exception(__FILE__,__LINE__);
		
	return itr->second;
}

Index::lookupid_t StringIndex::getLookupID(const char* key) const
{
	lookup_table_type::const_iterator itr=m_lookup_table.find(key);
	if (itr==m_lookup_table.end())
		return 0;
	return itr->second;
}

void StringIndex::del(docid_t docid)
{
	// Iterate the keys
	map_type::iterator itr_end=m_map.end();
	for(map_type::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		if (itr->second.test(docid)) // If the bit is set
		{
			itr->second.clr(docid);
			
			// Remove it from m_lookup_table
			lookup_table_type::const_iterator itr2_end=m_lookup_table.end();
			for (lookup_table_type::const_iterator itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
			{
				if (itr2->second==itr->first)
					m_lookup_table.erase(itr2->first);
			}
		}
	}
}

void StringIndex::load()
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
	std::ifstream ifs(m_filename.string().c_str());

	UIntIndex<uint32_t>::load_data(ifs);
	
	if (!m_lookup_table.empty())
		m_lookup_table.clear();
		
	if (ifs.good())
	{
		if (m_headerinfo.type==INDEX_TYPE_UNKNOWN && m_headerinfo.keycount==0)
			m_headerinfo.type=INDEX_TYPE_STRING;
		else if (m_headerinfo.type!=INDEX_TYPE_STRING)
			throw Exception(__FILE__,__LINE__);

		for(uint32_t i = 0; i < m_headerinfo.keycount; ++i)
		{
			// Read key
			char buf[256];
			size_t len;
			ifs.read((char*)&len,sizeof(len));
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);
			if (len>=sizeof(buf))
				throw Exception(__FILE__,__LINE__);
		
			ifs.read(buf,len);
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);

			buf[len]='\0';
			std::string key=buf;
			
			lookupid_t lookupid;
			ifs.read((char*)&lookupid,sizeof(lookupid));
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);

			// Put into index
			m_lookup_table.insert(make_pair(key,lookupid));
		}
	}
	
}

void StringIndex::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	UIntIndex<uint32_t>::save_data(ofs);
	
	size_t ckeys=0;
	
	// Write the lookup table
	lookup_table_type::const_iterator itr_end=m_lookup_table.end();
	for (lookup_table_type::const_iterator itr=m_lookup_table.begin(); itr!=itr_end; ++itr)
	{
		size_t len=itr->first.length();
		ofs.write((char*)&len, sizeof(len));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
		
		ofs.write(itr->first.c_str(),len);
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
		
		lookupid_t lookupid=itr->second;
		ofs.write((char*)&lookupid, sizeof(lookupid));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
			
		++ckeys;
	}
	
	if (ckeys!=m_headerinfo.keycount)
		throw Exception(__FILE__,__LINE__);
	
}

ostream& operator<<(ostream& os, const StringIndex& idx)
{
	os << dynamic_cast< const UIntIndex<uint32_t>& >(idx);
	
	// TODO: output lookup table

	return os;
}


}
