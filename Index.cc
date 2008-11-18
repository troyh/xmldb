#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{
	
DocSet Index::nil_docset(1); // Used to return "bad" DocSet from Index::get()

Index::key_t::key_t(const key_t& k) 
	: m_idx(k.m_idx)
{ 
	m_val=k.m_val;
}

bool   Index::key_t::operator< (const key_t& key) const
{
	if (m_idx!=key.m_idx)
		return false;
	
	return m_idx->compareKeys(*this,key) < 0;
}

bool Index::key_t::operator==(const key_t& k) const
{
	if (m_idx!=k.m_idx)
		return false;
		
	     if (!strcasecmp(m_idx->keyType(),"int8"))    { return m_val.int8  ==k.m_val.int8  ; }
	else if (!strcasecmp(m_idx->keyType(),"int16"))   { return m_val.int16 ==k.m_val.int16 ; }
	else if (!strcasecmp(m_idx->keyType(),"int32"))   { return m_val.int32 ==k.m_val.int32 ; }
	else if (!strcasecmp(m_idx->keyType(),"int64"))   { return m_val.int64 ==k.m_val.int64 ; }
	else if (!strcasecmp(m_idx->keyType(),"uint8"))   { return m_val.uint8 ==k.m_val.uint8 ; }
	else if (!strcasecmp(m_idx->keyType(),"uint16"))  { return m_val.uint16==k.m_val.uint16; }
	else if (!strcasecmp(m_idx->keyType(),"uint32"))  { return m_val.uint32==k.m_val.uint32; }
	else if (!strcasecmp(m_idx->keyType(),"uint64"))  { return m_val.uint64==k.m_val.uint64; }
	else if (!strcasecmp(m_idx->keyType(),"dbl"))     { return m_val.dbl   ==k.m_val.dbl   ; }
	else if (!strcasecmp(m_idx->keyType(),"char8"))   { return strncmp(m_val.ch, k.m_val.ch, sizeof(m_val.ch))==0; }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

void Index::key_t::output(ostream& os) const
{
	     if (!strcasecmp(m_idx->keyType(),"int8"))    { os << m_val.int8  ; }
	else if (!strcasecmp(m_idx->keyType(),"int16"))   { os << m_val.int16 ; }
	else if (!strcasecmp(m_idx->keyType(),"int32"))   { os << m_val.int32 ; }
	else if (!strcasecmp(m_idx->keyType(),"int64"))   { os << m_val.int64 ; }
	else if (!strcasecmp(m_idx->keyType(),"uint8"))   { os << m_val.uint8 ; }
	else if (!strcasecmp(m_idx->keyType(),"uint16"))  { os << m_val.uint16; }
	else if (!strcasecmp(m_idx->keyType(),"uint32"))  { os << m_val.uint32; }
	else if (!strcasecmp(m_idx->keyType(),"uint64"))  { os << m_val.uint64; }
	else if (!strcasecmp(m_idx->keyType(),"dbl"))     { os << m_val.dbl   ; }
	else if (!strcasecmp(m_idx->keyType(),"char8"))   { os.write(m_val.ch, sizeof(m_val.ch)); }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

int Index::compareKeys(const key_t& key1, const key_t& key2) const
{
	     if (!strcasecmp(keyType(),"int8"))    { return key1.m_val.int8   - key2.m_val.int8; }
	else if (!strcasecmp(keyType(),"int16"))   { return key1.m_val.int16  - key2.m_val.int16; }
	else if (!strcasecmp(keyType(),"int32"))   { return key1.m_val.int32  - key2.m_val.int32; }
	else if (!strcasecmp(keyType(),"int64"))   { return key1.m_val.int64  - key2.m_val.int64; }
	else if (!strcasecmp(keyType(),"uint8"))   { return key1.m_val.uint8  - key2.m_val.uint8 ; }
	else if (!strcasecmp(keyType(),"uint16"))  { return key1.m_val.uint16 - key2.m_val.uint16; }
	else if (!strcasecmp(keyType(),"uint32"))  { return key1.m_val.uint32 - key2.m_val.uint32; }
	else if (!strcasecmp(keyType(),"uint64"))  { return key1.m_val.uint64 - key2.m_val.uint64; }
	else if (!strcasecmp(keyType(),"dbl"))     { return key1.m_val.dbl    - key2.m_val.dbl   ; }
	else if (!strcasecmp(keyType(),"char8"))   { return strncmp(key1.m_val.ch, key2.m_val.ch, sizeof(key1.m_val.ch)); }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}

	return 0;
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

Index::key_t* Index::createKey() const
{
	     if (!strcasecmp(keyType(),"int8"))    { return new   int8key_t(this); }
	else if (!strcasecmp(keyType(),"int16"))   { return new  int16key_t(this); }
	else if (!strcasecmp(keyType(),"int32"))   { return new  int32key_t(this); }
	else if (!strcasecmp(keyType(),"int64"))   { return new  int64key_t(this); }
	else if (!strcasecmp(keyType(),"uint8"))   { return new  uint8key_t(this); }
	else if (!strcasecmp(keyType(),"uint16"))  { return new uint16key_t(this); }
	else if (!strcasecmp(keyType(),"uint32"))  { return new uint32key_t(this); }
	else if (!strcasecmp(keyType(),"uint64"))  { return new uint64key_t(this); }
	else if (!strcasecmp(keyType(),"dbl"))     { return new doublekey_t(this); }
	else if (!strcasecmp(keyType(),"char8"))   { return new  char8key_t(this); }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

void Index::put(const key_t& key, docid_t docid)
{
	// TODO: Force key to be the correct type
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

DocSet& Index::get(const key_t& key)
{
	map_iterator itr=m_map.find(key);
	if (itr==m_map.end())
		return nil_docset;
	return itr->second;
}

const DocSet& Index::get(const key_t& key) const
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
				key_t key(this);
				ifs.read(key,key.size());

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
		key_t key=itr->first;
		ofs.write(key,key.size());
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
	Iterator* itr    =((Index*)this)->begin();
	Iterator* itr_end=((Index*)this)->end();
	for (; *itr!=*itr_end; ++(*itr))
	{
		os << itr->key() << ':' << itr->docset() << endl;
	}
	
	delete itr_end;
	delete itr;
	
}

// void DateKey::output(ostream& os) const
// {
// 	struct tm* ptm=localtime((time_t*)&m_val.uint32);
// 	os << ptm->tm_year << '-' << ptm->tm_mon << '-' << ptm->tm_mday;
// }
// 
// void TimeKey::output(ostream& os) const
// {
// 	struct tm* ptm=localtime((time_t*)&m_val.uint32);
// 	os << ptm->tm_year << '-' << ptm->tm_mon << '-' << ptm->tm_mday << ' ' << ptm->tm_hour << ':' << ptm->tm_min << ':' << ptm->tm_sec;
// }
// 
// void FloatKey::output(ostream& os) const
// {
// 	os << m_val.dbl;
// }
// 
// void StringKey::output(ostream& os) const
// {
// 	os << m_s;
// }

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

ostream& operator<<(ostream& os, const Index::key_t& key) 
{ 
	key.output(os); 
	return os;
}

}
