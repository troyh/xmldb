#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

void StringIndex::put(const Key& key, docid_t docid)
{
	std::string keystr((const char*)key);
	const_iterator itr=m_lookup_table.find(keystr);
	if (itr==m_lookup_table.end())
	{ // Doesn't yet exist in index
	
		// Add it to the m_lookup_table by creating a lookupid
		// TODO: lock m_lookup_table
		size_type lookupid=m_lookup_table.size();
		m_lookup_table.insert(make_pair(keystr,lookupid));
		// TODO: unlock m_lookup_table
	
		Key idxkey=(uint32_t)lookupid;
		Index::put(idxkey,docid);
	}
	else
	{ // Update existing docset in index
		DocSet ds=Index::get(itr->second);
		if (ds.isNil())
			throw Exception(__FILE__,__LINE__);
			
		// DocSet& docset(itr2->second);
		ds.set(docid);
	}
	
}

DocSet& StringIndex::get(const Key& key)
{
	std::string keystr((const char*)key);
	const_iterator itr=m_lookup_table.find(keystr);
	if (itr==m_lookup_table.end())
	{
		return Index::nil_docset;
	}
	
	return Index::get(itr->second);
}

const DocSet& StringIndex::get(const Key& key) const
{
	return ((StringIndex*)(this))->get(key); // Cast away const-ness and use the non-const version
}

void StringIndex::del(docid_t docid)
{
	// Iterate the keys
	Iterator itr_end=Index::end();
	for(Iterator itr=Index::begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		if (itr.docset().test(docid)) // If the bit is set
		{
			itr.docset().clr(docid);
			
			// Remove it from m_lookup_table
			lookup_table_type::const_iterator itr2_end=m_lookup_table.end();
			for (lookup_table_type::const_iterator itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
			{
				if (itr2->second==itr.key())
					m_lookup_table.erase(itr2->first);
			}
		}
	}
}

void StringIndex::load()
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
	std::ifstream ifs(m_filename.string().c_str());

	Index::load_data(ifs);
	
	if (!m_lookup_table.empty())
		m_lookup_table.clear();
		
	if (ifs.good())
	{
		// if (m_headerinfo.type==INDEX_TYPE_UNKNOWN && m_headerinfo.keycount==0)
		// 	m_headerinfo.type=INDEX_TYPE_STRING;
		// else if (m_headerinfo.type!=INDEX_TYPE_STRING)
		// 	throw Exception(__FILE__,__LINE__);

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
			
			Key idxkey;
			ifs.read(idxkey,sizeof(idxkey));
			if (!ifs.good())
				throw Exception(__FILE__,__LINE__);

			// Put into index
			m_lookup_table.insert(make_pair(key,idxkey));
		}
	}
	
}

void StringIndex::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	Index::save_data(ofs);
	
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
		
		Key key=itr->second;
		ofs.write(key, sizeof(key));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
			
		++ckeys;
	}
	
	if (ckeys!=m_headerinfo.keycount)
		throw Exception(__FILE__,__LINE__);
	
}

void StringIndex::output(ostream& os) const
{
	// TODO: output m_lookup_table
	Index::output(os);
}


}
