#include <sstream>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include "OuzoDB.hpp"

// TODO: use locks when saving files
// TODO: read config file
// TODO: support more index types (Float, Int, multi-val, etc.)
// TODO: support queries
// TODO: make it thread-safe
// TODO: In DocSet, don't always allocate a vector and a dynamic_bitset, only the one you need
// TODO: version index file format

namespace Ouzo
{

	using namespace xercesc;

	DocSet::DocSet(size_t capacity)
		: m_type(bitmap), 
		m_docs_arr(new std::vector<docid_t>), 
		m_docs_bitmap(new bitset_type(capacity,0,BitmapAllocator<unsigned long>())),
		m_capacity(capacity)
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
		m_capacity=ds.m_capacity;
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
				throw Exception(__FILE__,__LINE__);
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
			std::vector<docid_t>::const_iterator itr_end=m_docs_arr->end();
			bool found=false;
			for (std::vector<docid_t>::const_iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
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
		else // Array type
		{
			// Remove from vector
			std::vector<docid_t>::iterator itr_end=m_docs_arr->end();
			for (std::vector<docid_t>::iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
			{
				if (*itr==docno)
				{
					m_docs_arr->erase(itr);
					break;
				}
			}
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
			throw Exception(__FILE__,__LINE__);
		
		switch (m_type)
		{
			case arr:
			{
				m_docs_arr->clear();

				std::vector<docid_t>::size_type n;
				is.read((char*)&n,sizeof(n));

				if (!is.good())
					throw Exception(__FILE__,__LINE__);
				
				for(size_t i = 0; i < n; ++i)
				{
					docid_t docid;
					is.read((char*)&docid,sizeof(docid));

					if (!is.good())
						throw Exception(__FILE__,__LINE__);
					
					m_docs_arr->push_back(docid);
				}

				break;
			}
			case bitmap:
			{
				BitmapAllocator<bitset_type::block_type>::size_type n;
				is.read((char*)&n,sizeof(n));
				
				if (!is.good())
					throw Exception(__FILE__,__LINE__);
				
				m_docs_bitmap->clear();
				m_docs_bitmap->resize(n);
				
				BitmapAllocator< bitset_type::block_type > pa=m_docs_bitmap->get_allocator();
				if (pa.sizeInBytes()<n)
					throw Exception(__FILE__,__LINE__);
					
				is.read(pa.startOfSpace(),n);
				
				if (!is.good())
					throw Exception(__FILE__,__LINE__);
				
				break;
			}
			default:
				break;
		}
		
		// Should we convert from one type of data structure to the other for efficiency?
		set_type besttype=mostEfficientType();
		if (besttype!=m_type)
		{
			convertToType(besttype);
		}
	}

	void DocSet::save(ostream& os) const
	{
		os.write((char*)&m_type,sizeof(m_type));

		switch (m_type)
		{
			case arr:
			{
				std::vector<docid_t>::size_type n=m_docs_arr->size();
				os.write((char*)&n,sizeof(n));
				
				for(std::vector<docid_t>::size_type i = 0; i < n; ++i)
				{
					docid_t docid=(*m_docs_arr)[i];
					os.write((char*)&docid,sizeof(docid));
				}
				break;
			}
			case bitmap:
			{
				BitmapAllocator< bitset_type::block_type > pa=m_docs_bitmap->get_allocator();
				BitmapAllocator< bitset_type::block_type >::size_type n=pa.sizeInBytes();
				
				os.write((char*)&n,sizeof(n));
				if (!os.good())
					throw Exception(__FILE__,__LINE__);
					
				os.write(pa.startOfSpace(),n);
				if (!os.good())
					throw Exception(__FILE__,__LINE__);
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
			{
				bool first=true;
				for (std::vector<docid_t>::const_iterator itr=ds.m_docs_arr->begin(); itr!=ds.m_docs_arr->end(); ++itr)
				{
					if (!first)
						os << ',';
					os << *itr;
					first=false;
				}
				break;
			}
			case DocSet::bitmap:
			{
				bool first=true;
				for (DocSet::bitset_type::size_type n=ds.m_docs_bitmap->find_first(); n!=DocSet::bitset_type::npos; n=ds.m_docs_bitmap->find_next(n))
				{
					if (!first)
						os << ',';
					os << n;
					first=false;
				}
			}
			break;
		}
		return os;
	}

	/**
	Based on capacity and the number of documents we're storing, choose the type.
	*/
	DocSet::set_type DocSet::mostEfficientType() const
	{
		return bitmap; // TODO: don't always return bitmap, do the math
	}
	
	void DocSet::convertToType(set_type t)
	{
		if (m_type!=t)
		{
			switch (t)
			{
			case bitmap:
			{
				// Convert from vector to bitmap
				std::vector<docid_t>::const_iterator itr_end=m_docs_arr->end();
				for (std::vector<docid_t>::const_iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
				{
					docid_t docid=*itr;
					m_docs_bitmap->set(docid-1);
				}
				
				m_docs_arr->clear();
				break;
			}
			case arr:
			{
				// Convert from bitmap to vector
				for (bitset_type::size_type n=m_docs_bitmap->find_first(); n!=bitset_type::npos; n=m_docs_bitmap->find_next(n))
				{
					m_docs_arr->push_back(n+1);
				}
				
				m_docs_bitmap->resize(0);
				break;
			}
			default:
				throw Exception(__FILE__,__LINE__);
				break;
			}
			
			m_type=t;
		}
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
		std::map<uint32_t,DocSet>::const_iterator itr=m_map.find(key);
		return itr->second;
	}
	
	void UIntIndex::del(docid_t docid)
	{
		// Iterate the keys
		std::map<uint32_t,DocSet>::iterator itr_end=m_map.end();
		for(std::map<uint32_t,DocSet>::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
		{
			// Remove the docid from the DocSet
			itr->second.clr(docid);
		}
	}
	
	void UIntIndex::load(boost::interprocess::file_lock& f_lock)
	{
		boost::interprocess::sharable_lock<file_lock> sh_lock(f_lock);
		
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
	
	void UIntIndex::save(boost::interprocess::file_lock& f_lock) const
	{
		boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
		
		std::ofstream ofs(m_filename.string().c_str());
		
		std::map<uint32_t,DocSet>::const_iterator itr_end=m_map.end();
		size_t ckeys=0;
		for(std::map<uint32_t,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
		{
			// Write key
			uint32_t n=itr->first;
			ofs.write((char*)&n,sizeof(n));

			// Write DocSet
			itr->second.save(ofs);
			++ckeys;
		}
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
		std::map<std::string,DocSet>::const_iterator itr=m_map.find(key);
		return itr->second;
	}
	
	void StringIndex::del(docid_t docid)
	{
		// Iterate the keys
		std::map<std::string,DocSet>::iterator itr_end=m_map.end();
		for(std::map<std::string,DocSet>::iterator itr=m_map.begin(); itr!=itr_end; ++itr)
		{
			// Remove the docid from the DocSet
			itr->second.clr(docid);
		}
	}
	
	void StringIndex::load(boost::interprocess::file_lock& f_lock)
	{
		sharable_lock<boost::interprocess::file_lock> sh_lock(f_lock);
		
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
	
	void StringIndex::save(boost::interprocess::file_lock& f_lock) const
	{
		boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);

		std::ofstream ofs(m_filename.string().c_str());
		
		std::map<std::string,DocSet>::const_iterator itr_end=m_map.end();
		for(std::map<std::string,DocSet>::const_iterator itr = m_map.begin(); itr != itr_end; ++itr)
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
		std::map<uint32_t,DocSet>::const_iterator itr_end=idx.m_map.end();
		for(std::map<uint32_t,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
		{
			const DocSet& ds=idx2.get(itr->first);
			os << itr->first << ':' << ds << endl;
		}
		return os;
	}
	
	ostream& operator<<(ostream& os, const StringIndex& idx)
	{
		std::map<std::string,DocSet>::const_iterator itr_end=idx.m_map.end();
		for(std::map<std::string,DocSet>::const_iterator itr = idx.m_map.begin(); itr != itr_end; ++itr)
		{
			os << itr->first << ':' << endl;
		}
		return os;
	}
	
	void Config::set(std::string name, std::string value)
	{
		m_info[name]=value;
	}
	
	void Config::set(std::string name, uint32_t value)
	{
		std::ostringstream n;
		n << value;
		m_info[name]=n.str();
	}
	
	Ouzo::Ouzo(bfs::path config_file)
		: m_config_file(config_file)
	{
		// TODO: read these from config file
		m_cfg.set("docdir","/home/troy/medline/docs/"); // MUST have a trailing slash!
		m_cfg.set("datadir",".");
		m_cfg.set("doccapacity",1000);
		
		Index* p=new UIntIndex("PMID.index","/MedlineCitation/PMID/text()");
		m_indexes.push_back(p);
		p=new UIntIndex("Year.index","/MedlineCitation/DateCreated/Year/text()");
		m_indexes.push_back(p);
		
		bfs::path datadir(m_cfg.get("datadir"));

		try
		{
			bfs::path fname=datadir / "docidmap";
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
			bfs::path fname=datadir / "docid.map";
			if (exists(fname))
			{
				std::ifstream ifs(fname.string().c_str());
				ifs >> m_avail_docids;
			}

			size_t capacity=strtoul(m_cfg.get("doccapacity").c_str(),0,10);
			if (m_avail_docids.size()<capacity)
				m_avail_docids.resize(capacity,1);
			
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
		return "NOTFOUND"; // TODO: return an appropriate error
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
			if(document == 0)
			{
		        std::cerr << "Document not found." << std::endl;
				throw Exception(__FILE__,__LINE__);
			}

			// Read config_file to get list of keys to index
			// loadConfig();
			
			// Iterate the indexes
			for (std::vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
			{
				Index* idx=m_indexes[i];
				
				boost::interprocess::file_lock f_lock(idx->filename().string().c_str());
				
				{
					boost::interprocess::sharable_lock<file_lock> sh_lock(f_lock);

					idx->load(f_lock);
				
					// First remove any existing data in the index for this docid
					idx->del(docid);
				
					// Parse an XPath 2 expression
			        const DOMXPathExpression* expression = document->createExpression(X(idx->keyspec().c_str()), 0);

			        // Execute the query
			        XPath2Result* result = (XPath2Result*)expression->evaluate(document, XPath2Result::ITERATOR_RESULT, 0);

			        // Iterate over the results
			        while(result->iterateNext())
					{
						const char* val=XMLString::transcode(result->asNode()->getTextContent());
						idx->put(val,docid);
						XMLString::release((char**)&val);
			        }
	
			        // Clean up all the objects we have created
			        result->release();
			        ((XQillaExpression*)expression)->release();

					// idx->merge(idx2);
					
					idx->save(f_lock);
				}
				
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
			boost::interprocess::file_lock f_lock(m_indexes[i]->filename().string().c_str());
			m_indexes[i]->save(f_lock);
		}
		
		bfs::path datadir(m_cfg.get("datadir"));
		
		// Write docidmap file
		try
		{
			bfs::path fname=datadir / "docidmap";
			std::ofstream ofs(fname.string().c_str());
			boost::archive::text_oarchive ar(ofs);
			boost::serialization::save(ar,m_docidmap,0);
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}
		
		bfs::path fname=datadir / "docid.map";
		std::ofstream ofs(fname.string().c_str());
		ofs << m_avail_docids;

		// TODO: Unlock docidmap file
		// TODO: unlock docid.map file
	}
	
	void Ouzo::addDocument(bfs::path docfile, doctype type)
	{
		std::string fname;
		
		// If docfile is an absolute path, verify that it is in the config's documents/dir
		if (docfile.has_root_path())
		{
			// Is it in the config's documents/dir?
			std::string s1=m_cfg.get("docdir");
			std::string s2=docfile.string().substr(0,s1.length());
			if (s1!=s2)
			{
				return; // TODO: return an appropriate error
			}
			else
			{
				fname=docfile.string().substr(s1.length());
			}
		}
		else // Relative path, pre-pend the config's documents/dir
		{
			fname=docfile.string();
			docfile=m_cfg.get("docdir") / docfile;
		}
		
		docid_t docid;
		// Find out if we already know about this document
		if (m_docidmap.find(fname)==m_docidmap.end())
		{
			// Create a new unique docid
			dynamic_bitset<>::size_type n=m_avail_docids.find_first(); // Find first on bit which represents the first available doc number
			if (n==dynamic_bitset<>::npos) // No available docid
			{
				throw Exception(__FILE__,__LINE__);
			}
			
			docid=n+1;
		}
		else
		{ 
			// Get the docid
			docid=m_docidmap[fname];
		}
		
		switch (type)
		{
			case XML:
				addXMLDocument(docfile, docid);
				break;
			default:
				break;
		}
		
		m_docidmap[fname]=docid;
		m_avail_docids.set(docid-1,false);
		
		persist();
	}
	
	void Ouzo::delDocument(bfs::path docfile)
	{
		bool changed=false;
		
		// Find out if we already know about this document
		if (m_docidmap.find(docfile)!=m_docidmap.end())
		{
			docid_t docid=m_docidmap[docfile];
			m_docidmap.erase(docfile);
			m_avail_docids.set(docid-1,true);
			
			// Iterate the indexes
			for (std::vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
			{
				Index* idx=m_indexes[i];
			
				boost::interprocess::file_lock f_lock(idx->filename().string().c_str());
				idx->load(f_lock);
				idx->del(docid);
				idx->save(f_lock);
			}
			
			changed=true;
		}
		
		if (changed)
			persist();
	}

	std::ostream& operator<<(std::ostream& os, const Ouzo& ouzo)
	{
		dynamic_bitset<>::size_type nextdocid=ouzo.m_avail_docids.find_first()+1;
		
		os << "Config file     :" << ouzo.m_config_file << std::endl
		   << "Doc dir         :" << ouzo.m_cfg.get("docdir") << std::endl
		   << "Data dir        :" << ouzo.m_cfg.get("datadir") << std::endl
		   << "Doc capacity    :" << ouzo.m_cfg.get("doccapacity") << std::endl
		   << "Next avail docid:" << nextdocid << std::endl
  		   << "Doc-ID map      :" << std::endl;

		std::map<bfs::path,docid_t>::const_iterator itr_end=ouzo.m_docidmap.end();
		for (std::map<bfs::path,docid_t>::const_iterator itr=ouzo.m_docidmap.begin(); itr!=itr_end; ++itr)
		{
			os << itr->second << '\t' << itr->first << std::endl;
		}
		os << std::endl;

		os << "Indexes:" << std::endl;
		for (std::vector<Index*>::size_type i=0; i< ouzo.m_indexes.size(); ++i)
		{
			Index* idx=ouzo.m_indexes[i];
			os << "------------------" << std::endl
			   << idx->filename() << std::endl
			   << "------------------" << std::endl;
			
			UIntIndex* uiidx=dynamic_cast<UIntIndex*>(idx);
			if (uiidx)
				os << *uiidx << std::endl;
			else
			{
				StringIndex* sidx=dynamic_cast<StringIndex*>(idx);
				if (sidx)
					os << *sidx << std::endl;
			}
		}
		
		return os;
	}
	
}

