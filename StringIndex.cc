#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

bool StringIndex::stringkey_t::operator< (const key_t& key) const
{
	if (m_idx!=key.getIndex())
		return false;

	return m_idx->compareKeys(*this,key) < 0;
}

Index::Iterator* StringIndex::begin()
{
	return new StringIterator(this,m_lookup_table.begin());
}

Index::Iterator* StringIndex::end()
{
	return new StringIterator(this,m_lookup_table.end());
}

Index::Iterator* StringIndex::lower_bound(const char* key)
{
	return new StringIterator(this,m_lookup_table.lower_bound(key));
}

Index::Iterator* StringIndex::lower_bound(const std::string& key)
{
	return new StringIterator(this,m_lookup_table.lower_bound(key));
}


void StringIndex::put(const key_t& key, docid_t docid)
{
	if (key.getIndex()!=this)
		throw Exception(__FILE__,__LINE__);
		
	std::string* s=(std::string*)key.m_val.ptr;
	if (!s)
		throw Exception(__FILE__,__LINE__);
	
	const_iterator itr=m_lookup_table.find(*s);
	if (itr==m_lookup_table.end())
	{ // Doesn't yet exist in index
	
		// Add it to the m_lookup_table by creating a lookupid
		// TODO: lock m_lookup_table

		uint32key_t idxkey(this,m_lookup_table.size());

		m_lookup_table.insert(make_pair(*s,idxkey));
		// TODO: unlock m_lookup_table
		
		Index::put(idxkey,docid);
		
		delete idxkey;
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

DocSet& StringIndex::get(const key_t& key)
{
	const stringkey_t* skey=dynamic_cast<const stringkey_t*>(&key);
	if (!skey)
		throw Exception(__FILE__,__LINE__);
		
	const std::string* s=skey->string();
	const_iterator itr=m_lookup_table.find(*s);
	if (itr==m_lookup_table.end())
	{
		return Index::nil_docset;
	}
	
	return Index::get(itr->second);
}

const DocSet& StringIndex::get(const key_t& key) const
{
	return ((StringIndex*)(this))->get(key); // Cast away const-ness and use the non-const version
}

void StringIndex::del(docid_t docid)
{
	// Iterate the keys
	Iterator* itr    =Index::begin();
	Iterator* itr_end=Index::end();
	for(; *itr!=*itr_end; ++(*itr))
	{
		// Remove the docid from the DocSet
		if (itr->docset().test(docid)) // If the bit is set
		{
			itr->docset().clr(docid);
			
			// Remove it from m_lookup_table
			lookup_table_type::const_iterator itr2_end=m_lookup_table.end();
			for (lookup_table_type::const_iterator itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
			{
				if (itr2->second==itr->key())
					m_lookup_table.erase(itr2->first);
			}
		}
	}
	
	delete itr_end;
	delete itr;
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
			
			key_t idxkey(this);
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
		
		key_t key=itr->second;
		ofs.write(key, key.size());
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
	lookup_table_type::const_iterator itr_end=m_lookup_table.end();
	for (lookup_table_type::const_iterator itr=m_lookup_table.begin(); itr!=itr_end; ++itr)
	{
		os << itr->first << ':';
		itr->second.output(os);
		os << endl;
	}
}

}
