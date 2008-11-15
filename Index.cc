#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{
	
DocSet Index::nil_docset(1); // Used to return "bad" DocSet from Index::get()

Key& Key::operator=(int8_t   x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%d",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(int16_t  x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%d",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(int32_t  x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%d",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(int64_t  x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%d",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(uint8_t  x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%u",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(uint16_t x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%u",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(uint32_t x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%u",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(uint64_t x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%u",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(double   x)
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)x;break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)x;break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)x;break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)x;break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)x;break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)x;break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)x;break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)x;break;
		case KEY_TYPE_DBL:    m_val.dbl   =(double)x;break;
		case KEY_TYPE_PTR:    m_val.ptr=0; break;
		case KEY_TYPE_CH:     
		{
			sprintf(m_val.ch,"%f",x);
			break;
		}
	}
	
	return *this;
}

Key& Key::operator=(const char*    x)     
{ 
	switch (m_type)
	{
		case KEY_TYPE_INT8:   m_val.int8  =  (int8_t)strtol(x,0,10);break;
		case KEY_TYPE_INT16:  m_val.int16 = (int16_t)strtol(x,0,10);break;
		case KEY_TYPE_INT32:  m_val.int32 = (int32_t)strtol(x,0,10);break;
		case KEY_TYPE_INT64:  m_val.int64 = (int64_t)strtol(x,0,10);break;
		case KEY_TYPE_UINT8:  m_val.uint8 = (uint8_t)strtoul(x,0,10);break;
		case KEY_TYPE_UINT16: m_val.uint16=(uint16_t)strtoul(x,0,10);break;
		case KEY_TYPE_UINT32: m_val.uint32=(uint32_t)strtoul(x,0,10);break;
		case KEY_TYPE_UINT64: m_val.uint64=(uint64_t)strtoul(x,0,10);break;
		case KEY_TYPE_DBL:    m_val.dbl   =strtof(x,0);break;
		case KEY_TYPE_CH:     memcpy(m_val.ch,x,sizeof(m_val.ch)); break;
		case KEY_TYPE_PTR:    m_val.ptr=(void*)x; break;
	}
	
	return *this;
}

Key& Key::operator=(void*	x)     { m_type=KEY_TYPE_PTR   ; m_val.ptr   =x;  return *this; } 

bool Key::operator<(const Key& key) const
{
	if (m_type!=key.m_type)
		return false;
		
	switch (m_type)
	{
		case KEY_TYPE_INT8:   return m_val.int8   < key.m_val.int8;   break;
		case KEY_TYPE_INT16:  return m_val.int16  < key.m_val.int16;  break;
		case KEY_TYPE_INT32:  return m_val.int32  < key.m_val.int32;  break;
		case KEY_TYPE_INT64:  return m_val.int64  < key.m_val.int64;  break;
		case KEY_TYPE_UINT8:  return m_val.uint8  < key.m_val.uint8;  break;
		case KEY_TYPE_UINT16: return m_val.uint16 < key.m_val.uint16; break;
		case KEY_TYPE_UINT32: return m_val.uint32 < key.m_val.uint32; break;
		case KEY_TYPE_UINT64: return m_val.uint64 < key.m_val.uint64; break;
		case KEY_TYPE_DBL:    return m_val.dbl    < key.m_val.dbl;    break;
		case KEY_TYPE_CH:     return m_val.ch     < key.m_val.ch;     break;
		case KEY_TYPE_PTR:    return m_val.ptr    < key.m_val.ptr;    break;
	}

}

void Key::output(ostream& os) const
{
	switch (m_type)
	{
		case KEY_TYPE_INT8:   os << m_val.int8  ; break;
		case KEY_TYPE_INT16:  os << m_val.int16 ; break;
		case KEY_TYPE_INT32:  os << m_val.int32 ; break;
		case KEY_TYPE_INT64:  os << m_val.int64 ; break;
		case KEY_TYPE_UINT8:  os << m_val.uint8 ; break;
		case KEY_TYPE_UINT16: os << m_val.uint16; break;
		case KEY_TYPE_UINT32: os << m_val.uint32; break;
		case KEY_TYPE_UINT64: os << m_val.uint64; break;
		case KEY_TYPE_DBL:    os << m_val.dbl   ; break;
		case KEY_TYPE_CH:     os << m_val.ch    ; break;
		case KEY_TYPE_PTR:    os << m_val.ptr   ; break;
	}
	
}



Index* Index::loadFromFile(bfs::path filename)
{
	Index* idx=new Index("","","",0);
	idx->setFilename(filename);
	idx->load();
	return idx;
}

	
Index::Index(const std::string& name, const key_type kt, const std::string& keyspec, uint32_t doccapacity)
	: m_name(name), m_keyspec(keyspec)
{
	memcpy(m_headerinfo.type,kt,sizeof(m_headerinfo.type));
	setFilename(name);
	if (!bfs::exists(m_filename))
	{
		ofstream f(m_filename.string().c_str()); // Create it
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
	
	m_headerinfo.doccount=0;
	m_headerinfo.keycount=0;
	m_headerinfo.keysize=0;
	m_headerinfo.doccapacity=doccapacity;
}

Index::~Index()
{
}

void Index::writeMeta(ostream& ofs) const
{
	VersionInfo verinfo;
	verinfo.version=FILEVERSION;
	verinfo.metasize=sizeof(m_headerinfo);
	
	ofs.write((char*)&verinfo,sizeof(verinfo));
	if (!ofs.good())
		throw Exception(__FILE__,__LINE__);

	ofs.write((char*)&m_headerinfo,sizeof(m_headerinfo));
	if (!ofs.good())
		throw Exception(__FILE__,__LINE__);

	ofs << m_keyspec << '\0';
	if (!ofs.good())
		throw Exception(__FILE__,__LINE__);
}

void Index::readMeta(istream& ifs)
{
	// Save the values that we've been configured with
	HeaderInfo config_info=m_headerinfo;
	std::string orig_keyspec=m_keyspec;
	
	VersionInfo verinfo;

	// Init meta info
	verinfo.version=0;
	verinfo.metasize=0;

	m_headerinfo.doccount=0;
	m_headerinfo.doccapacity=0;
	m_headerinfo.keyspeclen=0;
	m_headerinfo.keycount=0;
	m_headerinfo.keysize=0;
	memcpy(m_headerinfo.type,"unknown",sizeof(m_headerinfo.type));

	ifs.read((char*)&verinfo,sizeof(verinfo));
	if (!ifs.good())
	{
		verinfo.version=0;
		verinfo.metasize=0;
	}
	else
	{
		ifs.read((char*)&m_headerinfo,min(sizeof(m_headerinfo),verinfo.metasize));
		if (!ifs.good())
		{
			m_headerinfo.doccount=0;
			m_headerinfo.doccapacity=0;
			m_headerinfo.keyspeclen=0;
			m_headerinfo.keycount=0;
			m_headerinfo.keysize=0;
			memcpy(m_headerinfo.type,"unknown",sizeof(m_headerinfo.type));
		}
		else
		{
			if (verinfo.version >= 1)
			{
				getline(ifs,m_keyspec,'\0');
			}
		}
	}
	
	m_version=verinfo.version;
	// m_headerinfo.doccapacity=config_info.doccapacity; // Always ignore the capacity specified in the file and use the one specified at run-time

	// if (m_headerinfo.type==INDEX_TYPE_UNKNOWN)	// If the file doesn't have a type, take the type we're configured to have
	// 	m_headerinfo.type=getType(*this);
		
	// if (orig_keyspec!=m_keyspec)
	// {
	// 	// TODO: handle this discrepancy between the file's keyspec and the conf's keyspec
	// 	m_keyspec=orig_keyspec;
	// }
}

void Index::put(const char* s, docid_t docid)
{
	// Force key to be the correct type
	Key key=Ouzo::createKey(this);
	key=s;
	put(key,docid);
}

void Index::put(const Key& key, docid_t docid)
{
	map_iterator itr=m_map.find(key);
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

DocSet& Index::get(const Key& key)
{
	map_iterator itr=m_map.find(key);
	if (itr==m_map.end())
		return nil_docset;
	return itr->second;
}

const DocSet& Index::get(const Key& key) const
{
	const_map_iterator itr=m_map.find(key);
	if (itr==m_map.end())
		return nil_docset;
	return itr->second;
}

void Index::del(docid_t docid)
{
	// Iterate the keys
	map_iterator itr_end=m_map.end();
	for(map_iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		// Remove the docid from the DocSet
		itr->second.clr(docid);
	}
}


void Index::load()
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
	std::ifstream ifs(m_filename.string().c_str());
	
	load_data(ifs);
}

void Index::save() const
{
	Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),true);

	std::ofstream ofs(m_filename.string().c_str());
	
	save_data(ofs);
}

void Index::load_data(istream& ifs)
{
	// Clear out m_map
	if (!m_map.empty())
		m_map.clear();

	if (ifs.good())
	{
		readMeta(ifs);
		
		if (ifs.good())
		{
			// if (m_headerinfo.type==INDEX_TYPE_UNKNOWN && m_headerinfo.keycount==0)
			// 	m_headerinfo.type=INDEX_TYPE_UINT32;
			// Note: we can't expect INDEX_TYPE_UINT32, because this is a base for many similar types
			// else if (m_headerinfo.type!=INDEX_TYPE_UINT32)
			// 	throw Exception(__FILE__,__LINE__);

			for (uint32_t i=0;i<m_headerinfo.keycount;++i)
			{
				// Read key
				Key key;
				ifs.read(key,sizeof(key));

				if (!ifs.good())
					throw Exception(__FILE__,__LINE__);

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

void Index::save_data(ostream& ofs) const
{
	if (!ofs.good())
		throw Exception(__FILE__,__LINE__);

	// Write index meta info
	writeMeta(ofs);

	const_map_iterator itr_end=m_map.end();
	for(const_map_iterator itr = m_map.begin(); itr != itr_end; ++itr)
	{
		// Write key
		Key key=itr->first;
		ofs.write(key,sizeof(key));
		if (!ofs.good())
			throw Exception(__FILE__,__LINE__);

		// Write DocSet
		itr->second.save(ofs);
	}
}

void Index::output(ostream& os) const
{
	os	<< "Version  : " << version() << endl
		<< "Filename : " << m_filename << endl
		<< "Documents: " << documentCount() << endl
		<< "Capacity : " << documentCapacity() << endl
		<< "Key Type : " << m_headerinfo.type << endl
		<< "Key Spec : " << m_keyspec << endl
		<< "Key Count: " << keyCount() << endl
		<< "Key Size : " << m_headerinfo.keysize << endl
		<< endl;
		
	// Output keys
	Iterator itr_end=((Index*)this)->end();
	for (Iterator itr=((Index*)this)->begin(); itr!=itr_end; ++itr)
	{
		os << itr.key() << ':' << itr.docset() << endl;
	}
	
}

void DateKey::output(ostream& os) const
{
	struct tm* ptm=localtime((time_t*)&m_val.uint32);
	os << ptm->tm_year << '-' << ptm->tm_mon << '-' << ptm->tm_mday;
}

void TimeKey::output(ostream& os) const
{
	struct tm* ptm=localtime((time_t*)&m_val.uint32);
	os << ptm->tm_year << '-' << ptm->tm_mon << '-' << ptm->tm_mday << ' ' << ptm->tm_hour << ':' << ptm->tm_min << ':' << ptm->tm_sec;
}

void FloatKey::output(ostream& os) const
{
	os << m_val.dbl;
}

void StringKey::output(ostream& os) const
{
	os << m_s;
}

// ostream& operator<<(ostream& os, const Index::index_type t)
// {
// 	switch (t)
// 	{
// 		case Index::INDEX_TYPE_UNKNOWN: os << "unknown"; break;
// 		case Index::INDEX_TYPE_STRING:	os << "string";  break;
// 		case Index::INDEX_TYPE_UINT8:   os << "uint8";   break;
// 		case Index::INDEX_TYPE_UINT16:  os << "uint16";  break;
// 		case Index::INDEX_TYPE_UINT32:  os << "uint32";  break;
// 		case Index::INDEX_TYPE_FLOAT:   os << "float";   break;
// 		case Index::INDEX_TYPE_DATE:    os << "date";    break;
// 		case Index::INDEX_TYPE_TIME:    os << "time";    break;
// 		case Index::INDEX_TYPE_SINT8:   os << "sint8";   break;
// 		case Index::INDEX_TYPE_SINT16:  os << "sint16";  break;
// 		case Index::INDEX_TYPE_SINT32:  os << "sint32";  break;
// 		default:
// 			os << "???" << endl; // Shouldn't ever happen
// 	}
// 	
// 	return os;
// }


ostream& operator<<(ostream& os, const Index& idx)
{
	idx.output(os);
	return os;
}

}
