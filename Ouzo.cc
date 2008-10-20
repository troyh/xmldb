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
// #include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

#include "Ouzo.hpp"

// TODO: make StringIndex work
// TODO: support more index types (Float, Int, multi-val, etc.)
// TODO: support queries
// TODO: make it thread-safe
// TODO: In DocSet, don't always allocate a vector and a dynamic_bitset, only the one you need

namespace Ouzo
{

	using namespace xercesc;
	using namespace boost::interprocess;

	char* getXPathVal(const char* xpath, DOMDocument* document, const DOMNode* node)
	{
		char* ret=0;

		try
		{
			const DOMXPathExpression* expr=document->createExpression(X(xpath),0);
			if (expr)
			{
				XPath2Result* result=(XPath2Result*)expr->evaluate((DOMNode*)node, XPath2Result::ITERATOR_RESULT, 0);
				// The above casting away the const-ness seems wrong, why doesn't it take a const pointer??
				if (result)
				{
					if (result->iterateNext())
					{
						const DOMNode* pNode=result->asNode();
						switch (pNode->getNodeType())
						{
							case DOMNode::ELEMENT_NODE:
							{
								const DOMElement* pElem=dynamic_cast<const DOMElement*>(pNode);
								if (pElem)
									ret=XMLString::transcode(pElem->getTextContent());
								break;
							}
							case DOMNode::ATTRIBUTE_NODE:
							{
								const DOMAttr* pAttr=dynamic_cast<const DOMAttr*>(pNode);
								if (pAttr)
									ret=XMLString::transcode(pAttr->getValue());
								break;
							}
							case DOMNode::TEXT_NODE:
								break;
						}
					}
		
					result->release();
				}
			
		        ((XQillaExpression*)expr)->release();
			}
		}
		catch (XQillaException& x)
		{
			cout << XMLString::transcode(x.getString()) << endl;
		}
		
		return ret;
	}
	
	Ouzo::Ouzo(bfs::path config_file)
		: m_config_file(config_file)
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
			// TODO: validate against a DTD/XSchema

			// Parse a DOMDocument
			DOMDocument *document = builder->parseURI(X(m_config_file.string().c_str()));
			if(document == 0)
			{
				throw Exception(__FILE__,__LINE__);
			}
			else
			{
				char* s=getXPathVal("/ouzo/documents/dir",document,document);
				if (s)
				{
					m_cfg.set("docdir",	s);
					XMLString::release(&s);
				}
				// TODO: Add trailing slash to docdir
				
				s=getXPathVal("/ouzo/documents/capacity",document,document);
				if (s)
				{
					m_cfg.set("doccapacity",s);
					XMLString::release(&s);
				}
				
				s=getXPathVal("/ouzo/indexes/@directory",document,document);
				if (s)
				{
					m_cfg.set("datadir",s);
					XMLString::release(&s);
				}
				
				// Parse an XPath 2 expression
		        const DOMXPathExpression* expression = document->createExpression(X("/ouzo/indexes/index"), 0);
				if (expression)
				{
			        // Execute the query
			        XPath2Result* result = (XPath2Result*)expression->evaluate(document, XPath2Result::ITERATOR_RESULT, 0);
					if (result)
					{
				        // Iterate over the results
				        while (result->iterateNext())
						{
							const DOMElement* pElem=dynamic_cast<const DOMElement*>(result->asNode());
					
							const char* val=XMLString::transcode(pElem->getTextContent());
					
							const XMLCh* idxname=pElem->getAttribute(X("name"));
							const XMLCh* idxtype=pElem->getAttribute(X("type"));
							const XMLCh* idxuniq=pElem->getAttribute(X("unique")); // TODO: support this
							
							char* idxkey_s=getXPathVal("./xpath",document,pElem);

							char* idxname_s=XMLString::transcode(idxname);

							if (XMLString::equals(idxtype,X("string")))
							{
								Index* p=new StringIndex(idxname_s,idxkey_s,strtoul(m_cfg.get("doccapacity").c_str(),0,10));
								m_indexes.push_back(p);
							}
							else if (XMLString::equals(idxtype,X("uint32")))
							{
								Index* p=new UIntIndex(idxname_s,idxkey_s,strtoul(m_cfg.get("doccapacity").c_str(),0,10));
								m_indexes.push_back(p);
							}
					
							XMLString::release(&idxname_s);
							XMLString::release(&idxkey_s);
				        }

				        // Clean up all the objects we have created
				        result->release();
					}
					
			        ((XQillaExpression*)expression)->release();
				}

				builder->release();

			}
		}
		catch (...)
		{
			// Terminate Xerces-C and XQilla using XQillaPlatformUtils
			XQillaPlatformUtils::terminate();
			throw;
		}

		// Terminate Xerces-C and XQilla using XQillaPlatformUtils
		XQillaPlatformUtils::terminate();

		bfs::path datadir(m_cfg.get("datadir"));

		try
		{
			bfs::path fname=datadir / "docidmap";
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
	
	void Ouzo::persist()
	{
		// TODO: Lock docidmap file
		// TODO: lock docid.map file
			
		// Persist all the indexes
		for(size_t i = 0; i < m_indexes.size(); ++i)
		{
			m_indexes[i]->save();
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
	
	Index* Ouzo::getIndex(const bfs::path& fname)
	{
		std::vector<Index*>::const_iterator itr_end(m_indexes.end());
		for (std::vector<Index*>::const_iterator itr=m_indexes.begin(); itr!=itr_end; ++itr)
		{
			Index* pIdx=*itr;
			if (pIdx->filename()==fname)
			{
				return pIdx;
			}
		}
		return NULL;
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

		os << "Indexes (" << ouzo.m_indexes.size() << "):" << std::endl;
		for (std::vector<Index*>::size_type i=0; i< ouzo.m_indexes.size(); ++i)
		{
			Index* idx=ouzo.m_indexes[i];
			
			os << "------------------" << std::endl
			   << idx->filename() << " (" << idx->keyCount() << ")" << std::endl
			   << "------------------" << std::endl;
			
			idx->load();
		
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

