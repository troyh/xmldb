#include "Ouzo.hpp"

namespace Ouzo
{

Index::index_type Index::getType(const Index& idx)
{
	Index::index_type t=Index::INDEX_TYPE_UNKNOWN;
	
	if (dynamic_cast<const StringIndex*>(&idx))
		t=Index::INDEX_TYPE_STRING;
	else if (dynamic_cast<const FloatIndex*>(&idx))
		t=Index::INDEX_TYPE_FLOAT;
	else if (dynamic_cast<const DateIndex*>(&idx))
		t=Index::INDEX_TYPE_DATE;
	else if (dynamic_cast<const TimeIndex*>(&idx))
		t=Index::INDEX_TYPE_TIME;
	else if (dynamic_cast<const IntIndex<int8_t>* >(&idx))
		t=Index::INDEX_TYPE_SINT8;
	else if (dynamic_cast<const IntIndex<int16_t>* >(&idx))
		t=Index::INDEX_TYPE_SINT16;
	else if (dynamic_cast<const IntIndex<int32_t>* >(&idx))
		t=Index::INDEX_TYPE_SINT32;
	else if (dynamic_cast<const UIntIndex<uint8_t>* >(&idx))
		t=Index::INDEX_TYPE_UINT8;
	else if (dynamic_cast<const UIntIndex<uint16_t>* >(&idx))
		t=Index::INDEX_TYPE_UINT16;
	else if (dynamic_cast<const UIntIndex<uint32_t>* >(&idx))
		t=Index::INDEX_TYPE_UINT32;

	return t;
}
	
Index::Index(bfs::path index_file, const std::string& keyspec, uint32_t doccapacity)
	: m_filename(bfs::change_extension(index_file,".index")), m_keyspec(keyspec)
{
	if (!bfs::exists(m_filename))
	{
		ofstream f(m_filename.string().c_str()); // Create it
		if (!f.good())
			throw Exception(__FILE__,__LINE__);  // Unable to create it
	}
	
	m_headerinfo.doccapacity=doccapacity;
	m_headerinfo.type=getType(*this);
}

Index::~Index()
{
}

void Index::merge(const Index& other)
{
// 	// Add keys from other into this index
// 	Index::iterator iter_end=other.end();
// 	for (Index::iterator iter=other.begin();iter!=iter_end;++iter)
// 	{
// 		DocSet& docs=other[iter->first];
// 		// Remove all docs from other from this before adding documents
// 		for (uint32_t docno=docs.find_first();docno;docno=docs.find_next())
// 		{
// 			vector<Key>& keys=inv[docno];
// 			for(size_t i = 0; i < keys.size(); ++i)
// 			{
// 				*this[keys[i]].clr();
// 			}
// 		}
// 		
// 		Index::iterator tmp=this->find(iter->first);
// 		if (tmp==this->end()) // key not in index yet
// 		{
// 			this->insert(make_pair(iter->first,docs));
// 		}
// 		else // Add doc(s) to this key
// 		{
// 			DocSet& mydocs=*this[iter->first];
// 			mydocs|=docs;
// 		}
// 	}
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
	m_headerinfo.type=INDEX_TYPE_UNKNOWN;

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
			m_headerinfo.type=INDEX_TYPE_UNKNOWN;
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
	m_headerinfo.doccapacity=config_info.doccapacity; // Always ignore the capacity specified in the file and use the one specified at run-time

	if (m_headerinfo.type==INDEX_TYPE_UNKNOWN)	// If the file doesn't have a type, take the type we're configured to have
		m_headerinfo.type=getType(*this);
		
	if (orig_keyspec!=m_keyspec)
	{
		// TODO: handle this discrepancy between the file's keyspec and the conf's keyspec
		m_keyspec=orig_keyspec;
	}
}

ostream& operator<<(ostream& os, const Index::index_type t)
{
	switch (t)
	{
		case Index::INDEX_TYPE_UNKNOWN: os << "unknown"; break;
		case Index::INDEX_TYPE_STRING:	os << "string";  break;
		case Index::INDEX_TYPE_UINT8:   os << "uint8";   break;
		case Index::INDEX_TYPE_UINT16:  os << "uint16";  break;
		case Index::INDEX_TYPE_UINT32:  os << "uint32";  break;
		case Index::INDEX_TYPE_FLOAT:   os << "float";   break;
		case Index::INDEX_TYPE_DATE:    os << "date";    break;
		case Index::INDEX_TYPE_TIME:    os << "time";    break;
		case Index::INDEX_TYPE_SINT8:   os << "sint8";   break;
		case Index::INDEX_TYPE_SINT16:  os << "sint16";  break;
		case Index::INDEX_TYPE_SINT32:  os << "sint32";  break;
		default:
			os << "???" << endl; // Shouldn't ever happen
	}
	
	return os;
}

ostream& operator<<(ostream& os, const Index& idx)
{
	os	<< "Version  : " << idx.version() << endl
		<< "Filename : " << idx.m_filename << endl
		<< "Type	 : " << idx.m_headerinfo.type << endl
		<< "Documents: " << idx.documentCount() << endl
		<< "Capacity : " << idx.documentCapacity() << endl
		<< "Key Spec : " << idx.m_keyspec << endl
		<< "Key Count: " << idx.keyCount() << endl
		<< "Key Size : " << idx.m_headerinfo.keysize << endl
		<< endl;

	switch (Index::getType(idx))
	{
		case Index::INDEX_TYPE_UNKNOWN:
			break;
		case Index::INDEX_TYPE_STRING:
		{
			const StringIndex& rIdx=dynamic_cast<const StringIndex&>(idx);
			StringIndex::const_iterator_type itr_end=rIdx.end();
			for (StringIndex::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_UINT8:
		{
			const UIntIndex<uint8_t> rIdx=dynamic_cast< const UIntIndex<uint8_t>& >(idx);
			UIntIndex<uint8_t>::const_iterator_type itr_end=rIdx.end();
			for (UIntIndex<uint8_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_UINT16:
		{
			const UIntIndex<uint16_t> rIdx=dynamic_cast< const UIntIndex<uint16_t>& >(idx);
			UIntIndex<uint16_t>::const_iterator_type itr_end=rIdx.end();
			for (UIntIndex<uint16_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_UINT32:
		{
			const UIntIndex<uint32_t> rIdx=dynamic_cast< const UIntIndex<uint32_t>& >(idx);
			UIntIndex<uint32_t>::const_iterator_type itr_end=rIdx.end();
			for (UIntIndex<uint32_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_FLOAT:
		{
			const FloatIndex& rIdx=dynamic_cast<const FloatIndex&>(idx);
			FloatIndex::const_iterator_type itr_end=rIdx.end();
			for (FloatIndex::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_DATE:
		{
			const DateIndex& rIdx=dynamic_cast<const DateIndex&>(idx);
			DateIndex::const_iterator_type itr_end=rIdx.end();
			for (DateIndex::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_TIME:
		{
			const TimeIndex& rIdx=dynamic_cast<const TimeIndex&>(idx);
			TimeIndex::const_iterator_type itr_end=rIdx.end();
			for (TimeIndex::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_SINT8:
		{
			const IntIndex<int8_t>& rIdx=dynamic_cast< const IntIndex<int8_t>& >(idx);
			IntIndex<int8_t>::const_iterator_type itr_end=rIdx.end();
			for (IntIndex<int8_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_SINT16:
		{
			const IntIndex<int16_t>& rIdx=dynamic_cast< const IntIndex<int16_t>& >(idx);
			IntIndex<int16_t>::const_iterator_type itr_end=rIdx.end();
			for (IntIndex<int16_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
		case Index::INDEX_TYPE_SINT32:
		{
			const IntIndex<int32_t>& rIdx=dynamic_cast< const IntIndex<int32_t>& >(idx);
			IntIndex<int32_t>::const_iterator_type itr_end=rIdx.end();
			for (IntIndex<int32_t>::const_iterator_type itr=rIdx.begin(); itr!=itr_end; ++itr)
			{
				os << itr->first << "\t:" << itr->second << endl;
			}
			break;
		}
	}

	return os;
}

}
