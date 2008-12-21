#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

StringIndex::stringkey_t::stringkey_t(const char* s)
	: Index::key_t(Index::key_t::KEY_TYPE_STRING) 
{ 
	if (!s)
		throw Exception(__FILE__,__LINE__);

	m_val.ptr=new char[strlen(s)+1];
	if (!m_val.ptr)
		throw Exception(__FILE__,__LINE__);
		
	strcpy((char*)m_val.ptr,s);
}

StringIndex::stringkey_t::stringkey_t(const std::string& s)
	: Index::key_t(Index::key_t::KEY_TYPE_STRING) 
{ 
	m_val.ptr=new char[s.length()+1];
	if (!m_val.ptr)
		throw Exception(__FILE__,__LINE__);

	strcpy((char*)m_val.ptr,s.c_str());
}

StringIndex::stringkey_t::stringkey_t(const stringkey_t& k)
	: Index::key_t(Index::key_t::KEY_TYPE_STRING) 
{
	if (!k.m_val.ptr)
		throw Exception(__FILE__,__LINE__);
	m_val.ptr=new char[strlen((char*)k.m_val.ptr)+1];
	if (!m_val.ptr)
		throw Exception(__FILE__,__LINE__);

	strcpy((char*)m_val.ptr,(char*)k.m_val.ptr);
}

void StringIndex::stringkey_t::assign(const char* s)
{
	m_type=Index::key_t::KEY_TYPE_STRING;
	
	if (!s)
		throw Exception(__FILE__,__LINE__);

	m_val.ptr=new char[strlen(s)+1];
	if (!m_val.ptr)
		throw Exception(__FILE__,__LINE__);
		
	strcpy((char*)m_val.ptr,s);
}

bool StringIndex::stringkey_t::operator< (const key_t& key) const
{
	const stringkey_t* skey=dynamic_cast<const stringkey_t*>(&key);
	if (!skey)
	{
		if (key.getType()==KEY_TYPE_OBJECT)
		{
			if (key.m_val.object->getType()==KEY_TYPE_STRING)
			{
				skey=(stringkey_t*)key.m_val.object;
			}
			else
				throw Exception(__FILE__,__LINE__);
		}
		else if (key.getType()==KEY_TYPE_STRING)
		{
			// cout << "strcmp " << (char*)m_val.ptr << " and " << (char*)key.m_val.ptr << " value:" << (strcmp((char*)m_val.ptr,(char*)key.m_val.ptr)<0) << endl;
			return strcmp((char*)m_val.ptr,(char*)key.m_val.ptr)<0;
		}
		else
			throw Exception(__FILE__,__LINE__);
	}
	if (!skey)
		throw Exception(__FILE__,__LINE__);
		
	// cout << "StringIndex::stringkey_t::operator<() comparing " << (char*)m_val.ptr << " to " << (char*)skey->m_val.ptr << endl; 
	return strcmp((char*)m_val.ptr,(char*)skey->m_val.ptr)<0;
}

void StringIndex::stringkey_t::output(ostream& os) const
{
	os << (char*)m_val.ptr;
}

void StringIndex::stringkey_t::outputBinary(ostream& os,uint32_t n) const
{
	size_t len=strlen((char*)m_val.ptr);
	os.write((char*)&len,sizeof(len));
	if (!os.good())
		throw Exception(__FILE__,__LINE__);
		
	os.write((char*)m_val.ptr,len);
	if (!os.good())
		throw Exception(__FILE__,__LINE__);
}

void StringIndex::stringkey_t::inputBinary(istream& is)
{
	size_t len;
	is.read((char*)&len,sizeof(len));
	if (!is.good())
		throw Exception(__FILE__,__LINE__);

	m_val.ptr=new char[len+1];
	
	is.read((char*)m_val.ptr,len);
	if (!is.good())
		throw Exception(__FILE__,__LINE__);
	
	((char*)m_val.ptr)[len]='\0';
}

Index* StringIndex::createIndex(key_t::key_type kt, const char* name, const char* keyspec, uint32_t capacity)
{
	return new StringIndex(name,keyspec,capacity);
}

Index::key_t::key_type StringIndex::baseKeyType() const
{
	return key_t::KEY_TYPE_OBJECT;
}

void StringIndex::put(const key_t& key, docid_t docid)
{
	if (key.getType()!=Index::key_t::KEY_TYPE_STRING)
		throw Exception(__FILE__,__LINE__);
		
	const stringkey_t* skey=dynamic_cast<const stringkey_t*>(&key);
	if (!skey)
		throw Exception(__FILE__,__LINE__);
		
	// Copy the stringkey_t so the caller to this can destroy the one passed in
	stringkey_t* copyskey=new stringkey_t(*skey);
	// TODO: implement ref-counting in stringkey_t so we don't have to do this and the caller is free to destroy the key or not
	
	Index::key_t basekey(baseKeyType());
	basekey.assign((Index::key_t*)copyskey);
	Index::put(basekey,docid);
}

DocSet& StringIndex::get(const stringkey_t& key)
{
	Index::key_t basekey(baseKeyType());
	basekey.assign((Index::key_t*)&key);
	return Index::get(basekey);
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
		if (k.getType()!=Index::key_t::KEY_TYPE_OBJECT)
			throw Exception(__FILE__,__LINE__);
			
		delete k.m_val.object;
		
		deleted=true;
	}
	
	return deleted;
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
	
	// Write the table of contents
	map_type::const_iterator itr_end=Index::end();
	size_t pos=0;
	for (map_type::const_iterator itr=Index::begin(); itr!=itr_end; ++itr)
	{
		ofs.write((char*)&pos,sizeof(pos));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);
			
		Index::key_t k=itr->first;
		if (k.getType()!=Index::key_t::KEY_TYPE_OBJECT)
			throw Exception(__FILE__,__LINE__);
			
		stringkey_t* sk=dynamic_cast<stringkey_t*>(k.m_val.object);
		if (!sk)
			throw Exception(__FILE__,__LINE__);

		pos+=sizeof(size_t)+strlen(sk->c_str())+1+itr->second.sizeInBytes();
			
		++ckeys;
	}

	if (ckeys!=m_headerinfo.keycount)
		throw Exception(__FILE__,__LINE__);
	
}

}
