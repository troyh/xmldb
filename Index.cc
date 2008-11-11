#include <boost/interprocess/sync/file_lock.hpp>

#include "Ouzo.hpp"

namespace Ouzo
{

// Index::indexid_t Index::getID() const 
// {
// 	return (indexid_t)this;
// }

// Index::index_type Index::getType(const Index& idx)
// {
// 	Index::index_type t=Index::INDEX_TYPE_UNKNOWN;
// 	
// 	if (dynamic_cast<const StringIndex*>(&idx))
// 		t=Index::INDEX_TYPE_STRING;
// 	else if (dynamic_cast<const FloatIndex*>(&idx))
// 		t=Index::INDEX_TYPE_FLOAT;
// 	else if (dynamic_cast<const DateIndex*>(&idx))
// 		t=Index::INDEX_TYPE_DATE;
// 	else if (dynamic_cast<const TimeIndex*>(&idx))
// 		t=Index::INDEX_TYPE_TIME;
// 	else if (dynamic_cast<const IntIndex<int8_t>* >(&idx))
// 		t=Index::INDEX_TYPE_SINT8;
// 	else if (dynamic_cast<const IntIndex<int16_t>* >(&idx))
// 		t=Index::INDEX_TYPE_SINT16;
// 	else if (dynamic_cast<const IntIndex<int32_t>* >(&idx))
// 		t=Index::INDEX_TYPE_SINT32;
// 	else if (dynamic_cast<const UIntIndex<uint8_t>* >(&idx))
// 		t=Index::INDEX_TYPE_UINT8;
// 	else if (dynamic_cast<const UIntIndex<uint16_t>* >(&idx))
// 		t=Index::INDEX_TYPE_UINT16;
// 	else if (dynamic_cast<const UIntIndex<uint32_t>* >(&idx))
// 		t=Index::INDEX_TYPE_UINT32;
// 
// 	return t;
// }
// 

	
Index::Index(const std::string& name, bfs::path index_file, const std::string& keyspec, uint32_t doccapacity)
	: m_name(name), m_filename(bfs::change_extension(index_file,".index")), m_keyspec(keyspec)
{
	if (!bfs::exists(m_filename))
	{
		ofstream f(m_filename.string().c_str()); // Create it
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
	
	m_headerinfo.doccapacity=doccapacity;
	// m_headerinfo.type=getType(*this);
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
	// m_headerinfo.type=INDEX_TYPE_UNKNOWN;

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
			// m_headerinfo.type=INDEX_TYPE_UNKNOWN;
		}
		else
		{
			if (verinfo.version >= 1)
			{
				getline(ifs,m_keyspec,'\0');
			}
		}
	}
	
	// m_version=verinfo.version;
	m_headerinfo.doccapacity=config_info.doccapacity; // Always ignore the capacity specified in the file and use the one specified at run-time

	// if (m_headerinfo.type==INDEX_TYPE_UNKNOWN)	// If the file doesn't have a type, take the type we're configured to have
	// 	m_headerinfo.type=getType(*this);
		
	if (orig_keyspec!=m_keyspec)
	{
		// TODO: handle this discrepancy between the file's keyspec and the conf's keyspec
		m_keyspec=orig_keyspec;
	}
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

const DocSet& Index::get(const Key& key) const
{
	const_map_iterator itr=m_map.find(key);
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
		<< "Type	 : std" << endl
		<< "Documents: " << documentCount() << endl
		<< "Capacity : " << documentCapacity() << endl
		<< "Key Spec : " << m_keyspec << endl
		<< "Key Count: " << keyCount() << endl
		<< "Key Size : " << m_headerinfo.keysize << endl
		<< endl;
		
	// Output keys
	const_map_iterator itr_end=m_map.end();
	for (const_map_iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		os << (uint64_t)(itr->first) << ':' << itr->second << endl;
	}
	
}

void DateIndex::output(ostream& os) const
{
	os	<< "Version  : " << version() << endl
		<< "Filename : " << m_filename << endl
		<< "Type	 : date" << endl
		<< "Documents: " << documentCount() << endl
		<< "Capacity : " << documentCapacity() << endl
		<< "Key Spec : " << m_keyspec << endl
		<< "Key Count: " << keyCount() << endl
		<< "Key Size : " << m_headerinfo.keysize << endl
		<< endl;
		
	// Output keys
	const_map_iterator itr_end=m_map.end();
	for (const_map_iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		time_t t=(uint32_t)(itr->first);
		struct tm* ptm=localtime(&t);
		
		os << ptm->tm_year << '-' << ptm->tm_mon << '-' << ptm->tm_mday << ':' << itr->second << endl;
	}
}

void TimeIndex::output(ostream& os) const
{
	os	<< "Version  : " << version() << endl
		<< "Filename : " << m_filename << endl
		<< "Type	 : time" << endl
		<< "Documents: " << documentCount() << endl
		<< "Capacity : " << documentCapacity() << endl
		<< "Key Spec : " << m_keyspec << endl
		<< "Key Count: " << keyCount() << endl
		<< "Key Size : " << m_headerinfo.keysize << endl
		<< endl;
		
	// Output keys
	const_map_iterator itr_end=m_map.end();
	for (const_map_iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		time_t t=(uint32_t)(itr->first);
		os << ctime(&t) << ':' << itr->second << endl;
	}
}

void FloatIndex::output(ostream& os) const
{
	os	<< "Version  : " << version() << endl
		<< "Filename : " << m_filename << endl
		<< "Type	 : time" << endl
		<< "Documents: " << documentCount() << endl
		<< "Capacity : " << documentCapacity() << endl
		<< "Key Spec : " << m_keyspec << endl
		<< "Key Count: " << keyCount() << endl
		<< "Key Size : " << m_headerinfo.keysize << endl
		<< endl;
		
	// Output keys
	const_map_iterator itr_end=m_map.end();
	for (const_map_iterator itr=m_map.begin(); itr!=itr_end; ++itr)
	{
		os << (double)(itr->first) << ':' << itr->second << endl;
	}
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
