#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{
	
DocSet Index::nil_docset(1); // Used to return "bad" DocSet from Index::get()

Index::key_t::key_t(const key_t& k) 
	: m_type(k.m_type)
{ 
	m_val=k.m_val;
}

bool   Index::key_t::operator< (const key_t& key) const
{
	if (m_type!=key.m_type)
		throw Exception(__FILE__,__LINE__);

	     if (m_type==KEY_TYPE_INT8)    { return m_val.int8   < key.m_val.int8; }
	else if (m_type==KEY_TYPE_INT16)   { return m_val.int16  < key.m_val.int16; }
	else if (m_type==KEY_TYPE_INT32)   { return m_val.int32  < key.m_val.int32; }
	else if (m_type==KEY_TYPE_INT64)   { return m_val.int64  < key.m_val.int64; }
	else if (m_type==KEY_TYPE_UINT8)   { return m_val.uint8  < key.m_val.uint8 ; }
	else if (m_type==KEY_TYPE_UINT16)  { return m_val.uint16 < key.m_val.uint16; }
	else if (m_type==KEY_TYPE_UINT32)  { return m_val.uint32 < key.m_val.uint32; }
	else if (m_type==KEY_TYPE_UINT64)  { return m_val.uint64 < key.m_val.uint64; }
	else if (m_type==KEY_TYPE_DBL)     { return m_val.dbl    < key.m_val.dbl   ; }
	else if (m_type==KEY_TYPE_CHAR8)   { return strncmp(m_val.ch, key.m_val.ch, sizeof(m_val.ch)) < 0; }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

bool Index::key_t::operator==(const key_t& k) const
{
	if (m_type!=k.m_type)
		return false;
		
	     if (m_type==KEY_TYPE_INT8  )  { return m_val.int8  ==k.m_val.int8  ; }
	else if (m_type==KEY_TYPE_INT16 )  { return m_val.int16 ==k.m_val.int16 ; }
	else if (m_type==KEY_TYPE_INT32 )  { return m_val.int32 ==k.m_val.int32 ; }
	else if (m_type==KEY_TYPE_INT64 )  { return m_val.int64 ==k.m_val.int64 ; }
	else if (m_type==KEY_TYPE_UINT8 )  { return m_val.uint8 ==k.m_val.uint8 ; }
	else if (m_type==KEY_TYPE_UINT16)  { return m_val.uint16==k.m_val.uint16; }
	else if (m_type==KEY_TYPE_UINT32)  { return m_val.uint32==k.m_val.uint32; }
	else if (m_type==KEY_TYPE_UINT64)  { return m_val.uint64==k.m_val.uint64; }
	else if (m_type==KEY_TYPE_DBL   )  { return m_val.dbl   ==k.m_val.dbl   ; }
	else if (m_type==KEY_TYPE_CHAR8 )  { return strncmp(m_val.ch, k.m_val.ch, sizeof(m_val.ch))==0; }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

void Index::key_t::output(ostream& os) const
{
	     if (m_type==KEY_TYPE_INT8  )  { os << m_val.int8  ; }
	else if (m_type==KEY_TYPE_INT16 )  { os << m_val.int16 ; }
	else if (m_type==KEY_TYPE_INT32 )  { os << m_val.int32 ; }
	else if (m_type==KEY_TYPE_INT64 )  { os << m_val.int64 ; }
	else if (m_type==KEY_TYPE_UINT8 )  { os << m_val.uint8 ; }
	else if (m_type==KEY_TYPE_UINT16)  { os << m_val.uint16; }
	else if (m_type==KEY_TYPE_UINT32)  { os << m_val.uint32; }
	else if (m_type==KEY_TYPE_UINT64)  { os << m_val.uint64; }
	else if (m_type==KEY_TYPE_DBL   )  { os << m_val.dbl   ; }
	else if (m_type==KEY_TYPE_CHAR8 )  { os.write(m_val.ch, sizeof(m_val.ch)); }
	else
	{
		throw Exception(__FILE__,__LINE__);
	}
}

void Index::key_t::outputBinary(ostream& os) const
{
	os.write((char*)&m_val, sizeof(m_val));
}

void Index::key_t::inputBinary(istream& os)
{
	os.read((char*)&m_val, sizeof(m_val));
}

ostream& operator<<(ostream& os, Index::key_t::key_type t)
{
	switch (t)
	{
		case Index::key_t::KEY_TYPE_UNKNOWN:	os << "unknown"; break;
		case Index::key_t::KEY_TYPE_INT8	 :	os << "int8"   ; break;
		case Index::key_t::KEY_TYPE_INT16	 :	os << "int16"  ; break;
		case Index::key_t::KEY_TYPE_INT32	 :	os << "int32"  ; break;
		case Index::key_t::KEY_TYPE_INT64	 :	os << "int64"  ; break;
		case Index::key_t::KEY_TYPE_UINT8	 :	os << "uint8"  ; break;
		case Index::key_t::KEY_TYPE_UINT16 :	os << "uint16" ; break;
		case Index::key_t::KEY_TYPE_UINT32 :	os << "uint32" ; break;
		case Index::key_t::KEY_TYPE_UINT64 :	os << "uint64" ; break;
		case Index::key_t::KEY_TYPE_DBL	 :	os << "dbl"    ; break;
		case Index::key_t::KEY_TYPE_CHAR8	 :	os << "char8"  ; break;
		case Index::key_t::KEY_TYPE_PTR	 :	os << "ptr"    ; break;
		case Index::key_t::KEY_TYPE_DATE	 :	os << "date"   ; break;
		case Index::key_t::KEY_TYPE_TIME	 :	os << "time"   ; break;
		case Index::key_t::KEY_TYPE_FLOAT	 :	os << "float"  ; break;
		case Index::key_t::KEY_TYPE_STRING :	os << "string" ; break;
		default:
			throw new Exception(__FILE__,__LINE__); 
			break;
	}

	return os;
}


Index* Index::loadFromFile(bfs::path filename)
{
	VersionInfo verinfo;
	HeaderInfo headerinfo;

	// Init meta info
	verinfo.version=0;
	verinfo.metasize=0;

	headerinfo.doccount=0;
	headerinfo.doccapacity=0;
	headerinfo.keyspeclen=0;
	headerinfo.keycount=0;
	headerinfo.keysize=0;
	headerinfo.type=key_t::KEY_TYPE_UNKNOWN;

	{
	filename.replace_extension(".index");
	ifstream ifs(filename.string().c_str());
	ifs.read((char*)&verinfo,sizeof(verinfo));
	if (!ifs.good())
		throw Exception(__FILE__,__LINE__);
	ifs.read((char*)&headerinfo,min(sizeof(headerinfo),verinfo.metasize));
	if (!ifs.good())
		throw Exception(__FILE__,__LINE__);
	}

	Index* idx=Ouzo::createIndex(headerinfo.type,"","",0);
	if (!idx)
		throw Exception(__FILE__,__LINE__);
		
	idx->setFilename(filename);
	idx->load();
	
	return idx;
}

Index::key_t::key_type Index::key_t::getKeyType(const char* kt)
{
	     if (!strcasecmp(kt,"int8"  )) return KEY_TYPE_INT8  ;
	else if (!strcasecmp(kt,"int16" )) return KEY_TYPE_INT16 ;
	else if (!strcasecmp(kt,"int32" )) return KEY_TYPE_INT32 ;
	else if (!strcasecmp(kt,"int64" )) return KEY_TYPE_INT64 ;
	else if (!strcasecmp(kt,"uint8" )) return KEY_TYPE_UINT8 ;
	else if (!strcasecmp(kt,"uint16")) return KEY_TYPE_UINT16;
	else if (!strcasecmp(kt,"uint32")) return KEY_TYPE_UINT32;
	else if (!strcasecmp(kt,"uint64")) return KEY_TYPE_UINT64;
	else if (!strcasecmp(kt,"dbl"   )) return KEY_TYPE_DBL   ;
	else if (!strcasecmp(kt,"char8" )) return KEY_TYPE_CHAR8 ;
	else if (!strcasecmp(kt,"date"  )) return KEY_TYPE_DATE  ;
	else if (!strcasecmp(kt,"time"  )) return KEY_TYPE_TIME  ;
	else if (!strcasecmp(kt,"float" )) return KEY_TYPE_FLOAT ;
	else if (!strcasecmp(kt,"string")) return KEY_TYPE_STRING;

	return KEY_TYPE_UNKNOWN;
}

	
Index::Index(const std::string& name, key_t::key_type kt, const std::string& keyspec, uint32_t doccapacity)
	: m_name(name), m_keyspec(keyspec)
{
	m_headerinfo.doccount=0;
	m_headerinfo.keycount=0;
	m_headerinfo.keysize=0;
	m_headerinfo.doccapacity=doccapacity;
	m_headerinfo.type=kt;
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
	m_headerinfo.type=key_t::KEY_TYPE_UNKNOWN;

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
			m_headerinfo.type=key_t::KEY_TYPE_UNKNOWN;
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
	switch (keyType())
	{
		case key_t::KEY_TYPE_INT8  : return new int8key_t();   break;
		case key_t::KEY_TYPE_INT16 : return new int16key_t();  break;
		case key_t::KEY_TYPE_INT32 : return new int32key_t();  break;
		case key_t::KEY_TYPE_INT64 : return new int64key_t();  break;
		case key_t::KEY_TYPE_UINT8 : return new uint8key_t();  break;
		case key_t::KEY_TYPE_UINT16: return new uint16key_t(); break;
		case key_t::KEY_TYPE_UINT32: return new uint32key_t(); break;
		case key_t::KEY_TYPE_UINT64: return new uint64key_t(); break;
		case key_t::KEY_TYPE_DBL   : return new doublekey_t(); break;
		case key_t::KEY_TYPE_CHAR8 : return new char8key_t();  break;
		default:
			throw new Exception(__FILE__,__LINE__);
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

void Index::setFilename(bfs::path fname)
{
	m_filename=bfs::change_extension(fname,".index");

	if (!bfs::exists(m_filename))
	{
		ofstream f(m_filename.string().c_str(),ios::out);
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
}

void Index::load()
{
	if (bfs::exists(this->filename()))
	{
		Mutex<boost::interprocess::file_lock> mutex(this->filename().string(),false);
	
		std::ifstream ifs(m_filename.string().c_str());
	
		load_data(ifs);
	}
}

void Index::save() const
{
	if (!bfs::exists(this->filename()))
	{
		ofstream f(this->filename().string().c_str(),ios::out); // Make sure the file exists before trying to lock a mutex
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
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

			key_t::key_type basekt=baseKeyType();
			
			for (uint32_t i=0;i<m_headerinfo.keycount;++i)
			{
				// Read key
				Index::key_t key(basekt);
				key.inputBinary(ifs);

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
		key.outputBinary(ofs);
		
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
