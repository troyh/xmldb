#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

bool StringIndex::stringkey_t::operator< (const key_t& key) const
{
	if (m_type!=key.getType())
		return false;

	const stringkey_t& skey=dynamic_cast<const stringkey_t&>(key);
	return m_s < skey.m_s;
}

void StringIndex::stringkey_t::output(ostream& os) const
{
	os << m_s;
}

void StringIndex::stringkey_t::outputBinary(ostream& os) const
{
	size_t len=m_s.size()+1;
	os.write((char*)&len,sizeof(len));
	if (!os.good())
		throw Exception(__FILE__,__LINE__);
	os << m_s << '\0';
	if (!os.good())
		throw Exception(__FILE__,__LINE__);
}

void StringIndex::stringkey_t::inputBinary(istream& is)
{
	size_t len;
	is.read((char*)&len,sizeof(len));

	if (!is.good())
		throw Exception(__FILE__,__LINE__);
		
	m_s.resize(len);
	std::getline(is,m_s,'\0');
	if (!is.good())
		throw Exception(__FILE__,__LINE__);
}

Index* StringIndex::createIndex(key_t::key_type kt, const char* name, const char* keyspec, uint32_t capacity)
{
	return new StringIndex(name,keyspec,capacity);
}

Index::key_t::key_type StringIndex::baseKeyType() const
{
	return key_t::KEY_TYPE_UINT32;
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
	stringkey_t skey(key);
	return new StringIterator(this,m_lookup_table.lower_bound(skey));
}

Index::Iterator* StringIndex::lower_bound(const std::string& key)
{
	stringkey_t skey(key);
	return new StringIterator(this,m_lookup_table.lower_bound(skey));
}


void StringIndex::put(const key_t& key, docid_t docid)
{
	if (key.getType()!=Index::key_t::KEY_TYPE_STRING)
		throw Exception(__FILE__,__LINE__);
		
	const stringkey_t& skey=dynamic_cast<const stringkey_t&>(key);
		
	// std::string* s=(std::string*)key.m_val.ptr;
	// if (!s)
	// 	throw Exception(__FILE__,__LINE__);
	
	const_iterator itr=m_lookup_table.find(skey);
	if (itr==m_lookup_table.end())
	{ // Doesn't yet exist in index
	
		// Add it to the m_lookup_table by creating a lookupid
		// TODO: lock m_lookup_table

		key_t idxkey(baseKeyType());
		idxkey.assign((uint32_t)strtoul(skey.string().c_str(),0,10));

		m_lookup_table.insert(make_pair(skey,idxkey));
		// TODO: unlock m_lookup_table
		
		Index::put(idxkey,docid);
	}
	else
	{ // Update existing docset in index
		DocSet& ds=Index::get(itr->second);
		if (ds.isNil())
			throw Exception(__FILE__,__LINE__);
			
		ds.set(docid);
	}
	
}

DocSet& StringIndex::get(const stringkey_t& key)
{
	const_iterator itr=m_lookup_table.find(key);
	if (itr==m_lookup_table.end())
	{
		return Index::nil_docset;
	}
	
	return Index::get(itr->second);
}

const DocSet& StringIndex::get(const stringkey_t& key) const
{
	return ((StringIndex*)(this))->get(key); // Cast away const-ness and use the non-const version
}

bool StringIndex::del(docid_t docid, Index::key_t* pk)
{
	bool deleted=false;
	
	Index::key_t k;
	if (Index::del(docid,&k))
	{
		// Remove it from m_lookup_table
		lookup_table_type::iterator itr2_end=m_lookup_table.end();
		for (lookup_table_type::iterator itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
		{
			if (itr2->second==k)
			{
				m_lookup_table.erase(itr2);
				if (pk)
				{
					*pk=itr2->first;
				}
				deleted=true;
			}
		}
	}
	
	return deleted;
	
	// // Iterate the keys
	// Iterator* itr    =Index::begin();
	// Iterator* itr_end=Index::end();
	// for(; *itr!=*itr_end; ++(*itr))
	// {
	// 	// Remove the docid from the DocSet
	// 	if (itr->docset().test(docid)) // If the bit is set
	// 	{
	// 		itr->docset().clr(docid);
	// 
	// 		// Remove it from m_lookup_table
	// 		lookup_table_type::const_iterator itr2_end=m_lookup_table.end();
	// 		cout << "del:" << m_lookup_table.size() << endl;
	// 		for (lookup_table_type::const_iterator itr2=m_lookup_table.begin(); itr2!=itr2_end; ++itr2)
	// 		{
	// 			if (itr2->second==itr->key())
	// 				m_lookup_table.erase(itr2->first);
	// 		}
	// 	}
	// }
	// 
	// delete itr_end;
	// delete itr;
}

void StringIndex::load()
{
	if (bfs::exists(this->filename()))
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
				std::string s=buf;
				stringkey_t skey(s);
			
				Index::key_t idxkey(baseKeyType());
				idxkey.inputBinary(ifs);
				if (!ifs.good())
					throw Exception(__FILE__,__LINE__);

				// Put into index
				m_lookup_table.insert(make_pair(skey,idxkey));
			}
		}
	}
	
	if (m_lookup_table.size()!=m_headerinfo.keycount)
		throw Exception(__FILE__,__LINE__);
}

void StringIndex::save() const
{
	if (!bfs::exists(this->filename()))
	{
		ofstream f(this->filename().string().c_str(),ios::out); // Make sure the file exists before trying to lock a mutex
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	Index::save_data(ofs);
	
	size_t ckeys=0;
	
	// Write the lookup table
	lookup_table_type::const_iterator itr_end=m_lookup_table.end();
	for (lookup_table_type::const_iterator itr=m_lookup_table.begin(); itr!=itr_end; ++itr)
	{
		size_t len=itr->first.string().length();
		ofs.write((char*)&len, sizeof(len));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
		
		ofs.write(itr->first.string().c_str(),len);
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
		
		key_t key=itr->second;
		key.outputBinary(ofs);
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
			
		++ckeys;
	}

	if (ckeys!=m_headerinfo.keycount)
		throw Exception(__FILE__,__LINE__);
	
}


// void StringIndex::output(ostream& os) const
// {
// 	Index::output(os);
// 	lookup_table_type::const_iterator itr_end=m_lookup_table.end();
// 	for (lookup_table_type::const_iterator itr=m_lookup_table.begin(); itr!=itr_end; ++itr)
// 	{
// 		os << itr->first << ':';
// 		itr->second.output(os);
// 		os << endl;
// 	}
// }

}
