#include "Ouzo.hpp"

namespace Ouzo
{
	
Index::Index(bfs::path index_file, const std::string& keyspec, uint32_t doccapacity, index_type type)
	: m_filename(bfs::change_extension(index_file,".index")), m_keyspec(keyspec)
{
	if (!bfs::exists(m_filename))
	{
		ofstream f(m_filename.string().c_str()); // Create it
	}
	
	m_headerinfo.doccapacity=doccapacity;
	m_headerinfo.type=type;
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

	if (m_headerinfo.type==0)	// If the file doesn't have a type, take the type we're configured to have
		m_headerinfo.type=config_info.type;
}

ostream& operator<<(ostream& os, const Index::index_type t)
{
	switch (t)
	{
		case Index::INDEX_TYPE_UNKNOWN: os << "Unknown"; break;
		case Index::INDEX_TYPE_STRING:	os << "String";  break;
		case Index::INDEX_TYPE_UINT8:   os << "UInt8";   break;
		case Index::INDEX_TYPE_UINT16:  os << "UInt16";  break;
		case Index::INDEX_TYPE_UINT32:  os << "UInt32";  break;
		case Index::INDEX_TYPE_FLOAT:   os << "Float";   break;
		case Index::INDEX_TYPE_DATE:    os << "Date";    break;
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
	return os;
}

}
