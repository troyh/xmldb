#include <fstream>

#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include <xqilla/xqilla-dom3.hpp>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/path.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/dynamic_bitset.hpp>

#include "DocumentBase.hpp"
#include "UIntIndex.hpp"
#include "StringIndex.hpp"
#include "Exception.hpp"
#include "Mutex.hpp"


namespace Ouzo
{
	
	using namespace xercesc;
	
	void DocumentBase::docDirectory(const char* dir) 
	{
		m_docdir=dir; 
		m_docdir /= "foo"; // Temporarily add a filename so we can remove it and the trailing slash
		m_docdir=m_docdir.remove_filename().remove_filename();
		
	}
	void DocumentBase::docDirectory(bfs::path dir) 
	{
		m_docdir=dir; 
		m_docdir /= "foo"; // Temporarily add a filename so we can remove it and the trailing slash
		m_docdir=m_docdir.remove_filename().remove_filename();
	}

	void DocumentBase::dataDirectory(const char* dir) 
	{ 
		m_datadir=dir; 
		m_datadir /= "foo"; // Temporarily add a filename so we can remove it and the trailing slash
		m_datadir=m_datadir.remove_filename().remove_filename();
	}
	void DocumentBase::dataDirectory(bfs::path dir) 
	{ 
		m_datadir=dir; 
		m_datadir /= "foo"; // Temporarily add a filename so we can remove it and the trailing slash
		m_datadir=m_datadir.remove_filename().remove_filename();
	}
	
	void DocumentBase::load()
	{
		try
		{
			bfs::path fname=m_datadir / "docidmap";
			if (exists(fname))
			{
				std::ifstream ifs(fname.string().c_str());
				boost::archive::text_iarchive ar(ifs);
				boost::serialization::load(ar,m_docidmap,0);
			}
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}

		try
		{
			bfs::path fname=m_datadir / "docid.map";
			if (exists(fname))
			{
				std::ifstream ifs(fname.string().c_str());
				ifs >> m_avail_docids;
			}

			if (m_avail_docids.size()<m_capacity)
				m_avail_docids.resize(m_capacity,1);
		
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}
	}
	
	void DocumentBase::persist()
	{
		// TODO: Lock docidmap file
		// TODO: lock docid.map file

		// Persist all the indexes
		for(size_t i = 0; i < m_indexes.size(); ++i)
		{
			Index* idx=m_indexes[i];
			idx->save();
		}

		// Write docidmap file
		try
		{
			bfs::path fname=m_datadir / "docidmap";
			std::ofstream ofs(fname.string().c_str());
			boost::archive::text_oarchive ar(ofs);
			boost::serialization::save(ar,m_docidmap,0);
		}
		catch (boost::archive::archive_exception& x)
		{
			cout << "Archive exception: " << x.what() << endl;
		}

		bfs::path fname=m_datadir / "docid.map";
		std::ofstream ofs(fname.string().c_str());
		ofs << m_avail_docids;

		// TODO: Unlock docidmap file
		// TODO: unlock docid.map file
	}

	void DocumentBase::addDocument(bfs::path docfile)
	{
		std::string fname=docfile.filename();
		
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
		
		switch (m_fileformat)
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

	void DocumentBase::delDocument(bfs::path docfile)
	{
		std::string fname=docfile.filename();
		
		bool changed=false;
		
		// Find out if we already know about this document
		if (m_docidmap.find(fname)!=m_docidmap.end())
		{
			docid_t docid=m_docidmap[fname];
			m_docidmap.erase(docfile);
			m_avail_docids.set(docid-1,true);
			
			// Iterate the indexes
			for (std::vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
			{
				Index* idx=m_indexes[i];
			
				Mutex<boost::interprocess::file_lock> mutex(idx->filename().string(),true);
				
				idx->load();
				idx->del(docid);
				idx->save();
			}
			
			changed=true;
		}
		
		if (changed)
			persist();
	}

	void DocumentBase::addXMLDocument(bfs::path docfile, docid_t docid)
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

			// Iterate the indexes
			for (std::vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
			{
				Index* idx=m_indexes[i];

				Mutex<boost::interprocess::file_lock> mutex(idx->filename().string(),true); // Make sure no one else can read/write the index

				idx->load();
		
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
			
				idx->save();
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

	std::ostream& operator<<(std::ostream& os, const DocumentBase& doctype)
	{
		dynamic_bitset<>::size_type nextdocid=doctype.m_avail_docids.find_first()+1;
		
		os << "Doc dir         :" << doctype.docDirectory() << std::endl
		   << "Data dir        :" << doctype.dataDirectory() << std::endl
		   << "Doc capacity    :" << doctype.capacity() << std::endl
		   << "Next avail docid:" << nextdocid << std::endl
  		   << "Doc-ID map      :" << std::endl;

		std::map<bfs::path,docid_t>::const_iterator itr_end=doctype.m_docidmap.end();
		for (std::map<bfs::path,docid_t>::const_iterator itr=doctype.m_docidmap.begin(); itr!=itr_end; ++itr)
		{
			os << itr->second << '\t' << itr->first << std::endl;
		}
		os << std::endl;

		os << "Indexes (" << doctype.m_indexes.size() << "):" << std::endl;
		for (std::vector<Index*>::size_type i=0; i< doctype.m_indexes.size(); ++i)
		{
			Index* idx=doctype.m_indexes[i];
			
			idx->load();
		
			os << *idx << std::endl;
		}
		
		return os;
	}

	void DocumentBase::capacity(const char* str)
	{
		m_capacity=strtoul(str,0,10);
	}
	
	void DocumentBase::fileFormat(const char* s)
	{
		if (!strcasecmp(s,"XML"))
		{
			m_fileformat=XML;
		}
	}
	
	void DocumentBase::addIndex(Index* idx)
	{
		m_indexes.push_back(idx);
	}

	Index* DocumentBase::getIndex(std::string name)
	{
		for (std::vector<Index*>::size_type i=0; i< m_indexes.size(); ++i)
		{
			bfs::path idxname=m_indexes[i]->filename();
			idxname=bfs::change_extension(idxname.filename(),"");
			if (idxname==name)
				return m_indexes[i];
		}
		return NULL;
	}

}
