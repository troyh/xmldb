#include "OuzoDB.hpp"

namespace Ouzo
{

	using namespace xercesc;

	Semaphore::Semaphore()
	{
	}
	
	Semaphore::~Semaphore()
	{
	}
	
	DocSet::DocSet()
		: m_type(arr), m_docs_arr(new vector<docid_t>)
	{
	}
	
	DocSet::DocSet(DocSet& ds)
	{
		*this=ds;
	}

	DocSet::DocSet(const DocSet& ds)
	{
		*this=(DocSet&)ds;
	}
	
	DocSet& DocSet::operator=(DocSet& ds) 
	{ 
		m_docs_bitmap=ds.m_docs_bitmap; 
		m_docs_arr=ds.m_docs_arr; 
		m_type=ds.m_type; 
		return *this; 
	}

	DocSet& DocSet::operator=(const DocSet& ds) 
	{ 
		return *this=(DocSet&)ds; 
	}
	
	size_t DocSet::size() const
	{
		switch (m_type)
		{
			case arr:
				return m_docs_arr->size();
				break;
			case bitmap:
				return m_docs_bitmap->size();
				break;
			default:
				throw new exception; // TODO: throw Ouzo::Exception
				break;
		}
	}

	void DocSet::set(docid_t docno)
	{
		switch (this->type())
		{
		case bitmap:
			m_docs_bitmap->set(docno);
			break;
		case arr:
		{
			// See if it's already there
			vector<docid_t>::const_iterator itr_end=m_docs_arr->end();
			bool found=false;
			for (vector<docid_t>::const_iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
			{
				if (*itr==docno)
					found=true;
			}
			if (!found) // only add it if it's not already there
				m_docs_arr->push_back(docno);
			break;
		}
		default:
			throw Exception(__FILE__,__LINE__);
			break;
		}
	}
	
	void DocSet::clr(docid_t docno)
	{
		if (this->type()==bitmap)
		{
			m_docs_bitmap->reset(docno);
		}
		else
		{
			// TODO: remove from vector
		}
	}
	
	DocSet& DocSet::operator|=(const DocSet& ds)
	{
		if (this->type()==bitmap && ds.type()==bitmap) // Both are bitmaps
		{
			*(m_docs_bitmap)|=*(ds.m_docs_bitmap);
		}
		else
		{
			// TODO: combine manually
		}
		
		return *this;
	}

	DocSet& DocSet::operator&=(const DocSet& ds)
	{
		if (this->type()==bitmap && ds.type()==bitmap) // Both are bitmaps
		{
			*(m_docs_bitmap)&=*(ds.m_docs_bitmap);
		}
		else
		{
			// TODO: combine manually
		}
		
		return *this;
	}
	
	void DocSet::load(istream& is)
	{
		is.read((char*)&m_type,sizeof(m_type));
		
		if (!is.good())
			throw new exception; // TODO: throw Ouzo::Exception
		
		switch (m_type)
		{
			case arr:
			{
				m_docs_arr->clear();

				vector<docid_t>::size_type n;
				is.read((char*)&n,sizeof(n));

				if (!is.good())
					throw new exception; // TODO: throw Ouzo::Exception
				
				for(size_t i = 0; i < n; ++i)
				{
					docid_t docid;
					is.read((char*)&docid,sizeof(docid));

					if (!is.good())
						throw new exception; // TODO: throw Ouzo::Exception
					
					m_docs_arr->push_back(docid);
				}

				break;
			}
			case bitmap:
			{
				dynamic_bitset<>::size_type n;
				is.read((char*)&n,sizeof(n));

				if (!is.good())
					throw new exception; // TODO: throw Ouzo::Exception
				
				m_docs_bitmap->clear();
				m_docs_bitmap->resize(n);
				is >> *m_docs_bitmap;
				
				if (!is.good())
					throw new exception; // TODO: throw Ouzo::Exception
				
				break;
			}
			default:
				break;
		}
	}

	void DocSet::save(ostream& os) const
	{
		os.write((char*)&m_type,sizeof(m_type));

		switch (m_type)
		{
			case arr:
			{
				vector<docid_t>::size_type n=m_docs_arr->size();
				os.write((char*)&n,sizeof(n));
				
				for(vector<docid_t>::size_type i = 0; i < n; ++i)
				{
					docid_t docid=(*m_docs_arr)[i];
					os.write((char*)&docid,sizeof(docid));
				}
				break;
			}
			case bitmap:
			{
				dynamic_bitset<>::size_type n=m_docs_bitmap->size();
				os.write((char*)&n,sizeof(n));
				
				os << *m_docs_bitmap;
				os << '$';
				break;
			}
			default:
				break;
		}
	}

	ostream& operator<<(ostream& os, const DocSet& ds)
	{
		switch (ds.m_type)
		{
			case DocSet::arr:
				for (vector<docid_t>::const_iterator itr=ds.m_docs_arr->begin(); itr!=ds.m_docs_arr->end(); ++itr)
				{
					os << *itr << ',';
				}
			break;
			case DocSet::bitmap:
			break;
		}
		return os;
	}

	
	Index::Index(bfs::path index_file, const std::string& keyspec)
		: m_filename(index_file), m_keyspec(keyspec)
	{
	}
	
	Index::~Index()
	{
	}
	
	size_t Index::documentCount() const
	{
	}

	Semaphore Index::lock()
	{
	}

	void Index::unlock(const Semaphore& sem)
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
	
	void UIntIndex::put(const char* key, docid_t docid)
	{
		uint32_t val=strtoul(key,0,10);
		
		if (m_map.find(val)==m_map.end())
		{ // Doesn't yet exist in index
			DocSet docset;
			docset.set(docid);
			
			m_map.insert(make_pair(val,docset));
		}
		else
		{ // Update existing docset in index
			DocSet& docset=m_map[val];
			docset.set(docid);
		}
	}
	
	const DocSet& UIntIndex::get(uint32_t key) const
	{
		map<uint32_t,DocSet>::const_iterator itr=m_map.find(key);
		return itr->second;
	}
	
	void UIntIndex::load()
	{
		// Clear out m_map
		if (!m_map.empty())
			m_map.clear();
			
		std::ifstream ifs(m_filename.string().c_str());
		
		while (ifs.good())
		{
			// Read key
			uint32_t n;
			ifs.read((char*)&n,sizeof(n));

			if (ifs.good())
			{
				// Read DocSet
				DocSet ds;
				ds.load(ifs);

				// Put into index
				m_map.insert(make_pair(n,ds));
			}
		}
	}
	
	void UIntIndex::save() const
	{
		cout << "Saving index:" << m_filename << "..." << endl;
		std::ofstream ofs(m_filename.string().c_str());
		
		map<uint32_t,DocSet>::const_iterator itr_end=m_map.end();
		size_t ckeys=0;
		for(map<uint32_t,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
		{
			// Write key
			uint32_t n=itr->first;
			ofs.write((char*)&n,sizeof(n));

			// Write DocSet
			itr->second.save(ofs);
			++ckeys;
		}
		cout << "Wrote " << ckeys << " keys" << endl;
	}

	void StringIndex::put(const char* key, docid_t docid)
	{
		if (m_map.find(key)==m_map.end())
		{ // Doesn't yet exist in index
			DocSet docset;
			docset.set(docid);
			m_map.insert(make_pair(key,docset));
		}
		else
		{ // Update existing docset in index
			DocSet& docset=m_map[key];
			docset.set(docid);
		}
		
	}

	const DocSet& StringIndex::get(const char* key) const
	{
		map<std::string,DocSet>::const_iterator itr=m_map.find(key);
		return itr->second;
	}
	
	void StringIndex::load()
	{
		// Clear out m_map
		if (!m_map.empty())
			m_map.clear();
			
		std::ifstream ifs(m_filename.string().c_str());
		
		while (ifs.good())
		{
			// Read key
			std::string key;
			char buf[256];
			size_t len;
			ifs.read((char*)&len,sizeof(len));
			ifs.read(buf,len);
			buf[len]='\0';
			key=buf;
		
			// Read DocSet
			DocSet ds;
			ds.load(ifs);

			// Put into index
			m_map.insert(make_pair(key,ds));
		}
	}
	
	void StringIndex::save() const
	{
		std::ofstream ofs(m_filename.string().c_str());
		
		map<std::string,DocSet>::const_iterator itr_end=m_map.end();
		for(map<std::string,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
		{
			// Write key
			std::string key=itr->first;
			size_t len=key.size();
			
			ofs.write((char*)&len,sizeof(len));
			ofs.write(key.c_str(),len);
			
			// Write DocSet
			itr->second.save(ofs);
		}
	}
	
	ostream& operator<<(ostream& os, const UIntIndex& idx)
	{
		UIntIndex& idx2=(UIntIndex&)idx; // cast away const-ness because C++ is kinda dumb this way
		map<uint32_t,DocSet>::const_iterator itr_end=idx.m_map.end();
		for(map<uint32_t,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
		{
			const DocSet& ds=idx2.get(itr->first);
			os << itr->first << ':' << ds << endl;
		}
		return os;
	}
	
	ostream& operator<<(ostream& os, const StringIndex& idx)
	{
		map<std::string,DocSet>::const_iterator itr_end=idx.m_map.end();
		for(map<std::string,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
		{
			os << itr->first << ':' << endl;
		}
		return os;
	}
	
	Ouzo::Ouzo(bfs::path config_file)
		: m_config_file(config_file)
	{
		// TODO: read these from config file
		m_cfg_docdir="/home/troy/medline/docs";
		m_cfg_datadir=".";
		m_cfg_doccapacity=1000;
		
		Index* p=new UIntIndex("PMID.index","/MedlineCitation/PMID/text()");
		m_indexes.push_back(p);
		p=new UIntIndex("Year.index","/MedlineCitation/DateCreated/Year/text()");
		m_indexes.push_back(p);
		
		try
		{
			bfs::path fname=m_cfg_datadir / "docidmap";
			std::ifstream ifs(fname.string().c_str());
			boost::archive::text_iarchive ar(ifs);
			boost::serialization::load(ar,m_docidmap,0);
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}

		try
		{
			bfs::path fname=m_cfg_datadir / "docid.map";
			if (exists(fname))
			{
				std::ifstream ifs(fname.string().c_str());
				ifs >> m_avail_docids;
			}

			if (m_avail_docids.size()<m_cfg_doccapacity)
				m_avail_docids.resize(m_cfg_doccapacity,1);
			
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}
		
	}
	
	Ouzo::~Ouzo()
	{
	}
	
	const char* Ouzo::getNodeValue(const DOMNode* node,const char* tag)
	{
		for (DOMNode* kid=node->getFirstChild();kid;kid=kid->getNextSibling())
		{
			char* s=XMLString::transcode(kid->getNodeName());
			XMLString::release(&s);

			if (XMLString::equals(kid->getNodeName(),X(tag)))
			{
				return XMLString::transcode(kid->getTextContent());
			}
		}
		return "NOTFOUND";
	}

	void Ouzo::addXMLDocument(bfs::path docfile, docid_t docid)
	{
		// Initialise Xerces-C and XQilla using XQillaPlatformUtils
		XQillaPlatformUtils::initialize();

		try
		{
			// Get the XQilla DOMImplementation object
			DOMImplementation *xqillaImplementation=DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

			// Create a DOMBuilder object
			DOMBuilder *builder = xqillaImplementation->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
			builder->setFeature(X("namespaces"), true);
			builder->setFeature(X("http://apache.org/xml/features/validation/schema"), true);
			builder->setFeature(X("validation"), true);

			// Parse a DOMDocument
			DOMDocument *document = builder->parseURI(X(docfile.string().c_str()));
			if(document == 0) {
		        std::cerr << "Document not found." << std::endl;
				throw new exception();
			}

			// Read config_file to get list of keys to index
			// loadConfig();
			
			// Iterate the indexes
			for (vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
			{
				Index* idx=m_indexes[i];
				
				cout << "Index:" << idx->filename() << ", keyspec=" << idx->keyspec() << endl;
				Semaphore sem=idx->lock();
				
				idx->load();
				cout << idx->filename() << " before:" << (UIntIndex&)*idx << endl;
				
				// Parse an XPath 2 expression
		        const DOMXPathExpression* expression = document->createExpression(X(idx->keyspec().c_str()), 0);

		        // Execute the query
		        XPath2Result* result = (XPath2Result*)expression->evaluate(document, XPath2Result::ITERATOR_RESULT, 0);

		        // Iterate over the results
		        while(result->iterateNext()) {
					const char* val=XMLString::transcode(result->asNode()->getTextContent());
					idx->put(val,docid);
					XMLString::release((char**)&val);
		        }
	
		        // Clean up all the objects we have created
		        result->release();
		        ((XQillaExpression*)expression)->release();

				// idx->merge(idx2);
				
				idx->save();
				cout << idx->filename() << " after:" << (UIntIndex&)*idx << endl;
				
				idx->unlock(sem);
			}

			builder->release();

		}
		catch (...)
		{
			// Terminate Xerces-C and XQilla using XQillaPlatformUtils
			XQillaPlatformUtils::terminate();
			throw;
		}

		// Terminate Xerces-C and XQilla using XQillaPlatformUtils
		XQillaPlatformUtils::terminate();
		
	}
	
	void Ouzo::persist()
	{
		// TODO: Lock docidmap file
		// TODO: lock docid.map file
			
		// Persist all the indexes
		for(size_t i = 0; i < m_indexes.size(); ++i)
		{
			m_indexes[i]->save();
		}
		
		// Write docidmap file
		try
		{
			bfs::path fname=m_cfg_datadir / "docidmap";
			std::ofstream ofs(fname.string().c_str());
			boost::archive::text_oarchive ar(ofs);
			boost::serialization::save(ar,m_docidmap,0);
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}
		
		bfs::path fname=m_cfg_datadir / "docid.map";
		std::ofstream ofs(fname.string().c_str());
		ofs << m_avail_docids;

		// TODO: Unlock docidmap file
		// TODO: unlock docid.map file
	}
	
	void Ouzo::addDocument(bfs::path docfile, doctype type)
	{
		docid_t docid;
		// Find out if we already know about this document
		if (m_docidmap.find(docfile)==m_docidmap.end())
		{
			// Create a new unique docid
			dynamic_bitset<>::size_type n=m_avail_docids.find_first(); // Find first on bit which represents the first available doc number
			if (n==dynamic_bitset<>::npos)
			{
				throw new exception(); // TODO: throw Ouzo::Exception
			}
			
			docid=n+1;
		}
		else
		{ 
			// Get the docid
			docid=m_docidmap[docfile];
		}
		
		switch (type)
		{
			case XML:
				addXMLDocument(docfile, docid);
				break;
			default:
				break;
		}
		
		m_docidmap[docfile]=docid;
		m_avail_docids.set(docid-1,false);
		
		persist();
	}
	
	void Ouzo::delDocument(bfs::path docfile)
	{
		bool changed=false;
		
		// TODO: remove all references to this doc in m_docidmap and in indexes

		// Find out if we already know about this document
		if (m_docidmap.find(docfile)!=m_docidmap.end())
		{
			docid_t docid=m_docidmap[docfile];
			m_avail_docids.set(docid-1,true);
			changed=true;
		}
		
		if (changed)
			persist();
	}
	
}

