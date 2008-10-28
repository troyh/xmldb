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

#include "Ouzo.hpp"

// TODO: For unique indexes, make DocSet efficient, i.e., store only one docid, not a bitset or vector
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
	
	void Ouzo::readConfigIndexes(DOMDocument* document, const DOMElement* node, DocumentBase& doctype)
	{
		DOMNodeList* indexes=node->getElementsByTagName(X("indexes"));
		DOMElement* indexeselem=dynamic_cast<DOMElement*>(indexes->item(0)); // Should only be 1
		char* p=XMLString::transcode(indexeselem->getAttribute(X("directory")));
		doctype.dataDirectory(p);
		XMLString::release(&p);
		
		if (!bfs::exists(doctype.dataDirectory()))
			throw Exception(__FILE__,__LINE__);
		
		// Parse an XPath 2 expression
        const DOMXPathExpression* expression = document->createExpression(X("indexes/index"), 0);
		if (expression)
		{
	        // Execute the query
	        XPath2Result* result = (XPath2Result*)expression->evaluate((DOMElement*)node, XPath2Result::ITERATOR_RESULT, 0);
			if (result)
			{
		        // Iterate over the results
		        while (result->iterateNext())
				{
					const DOMElement* pElem=dynamic_cast<const DOMElement*>(result->asNode());
			
					const XMLCh* idxname=pElem->getAttribute(X("name"));
					const XMLCh* idxtype=pElem->getAttribute(X("type"));
					const XMLCh* idxuniq=pElem->getAttribute(X("unique")); // TODO: support this
					
					char* idxkey_s=getXPathVal("./key",document,pElem);
					if (idxkey_s)
					{
						char* idxname_s=XMLString::transcode(idxname);
						bfs::path idxpath(doctype.dataDirectory());
						idxpath /= idxname_s;
						XMLString::release(&idxname_s);
						
						Index* p;
					
						if (XMLString::equals(idxtype,X("string")))
						{
							p=new StringIndex(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("uint32")))
						{
							p=new UIntIndex<uint32_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("uint16")))
						{
							p=new UIntIndex<uint16_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("uint8")))
						{
							p=new UIntIndex<uint8_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("sint32")))
						{
							p=new IntIndex<int32_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("sint16")))
						{
							p=new IntIndex<int16_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("sint8")))
						{
							p=new IntIndex<int8_t>(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("date")))
						{
							p=new DateIndex(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("time")))
						{
							p=new TimeIndex(idxpath,idxkey_s,doctype.capacity());
						}
						else if (XMLString::equals(idxtype,X("float")))
						{
							p=new FloatIndex(idxpath,idxkey_s,doctype.capacity());
						}
						
						doctype.addIndex(p);
						
						XMLString::release(&idxkey_s);
					}
		        }

		        // Clean up all the objects we have created
		        result->release();
			}
			
	        ((XQillaExpression*)expression)->release();
		}
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
		        const XQillaExpression* expression = (XQillaExpression*)document->createExpression(X("/ouzo/documents"), 0);
				if (expression)
				{
			        XPath2Result* result = (XPath2Result*)expression->evaluate(document, XPath2Result::ITERATOR_RESULT, 0);
					if (result)
					{
				        // Iterate over the results
				        while (result->iterateNext())
						{
							const DOMElement* docselem=dynamic_cast<const DOMElement*>(result->asNode());

							DocumentBase* doctype=new DocumentBase();
							
							char* p=XMLString::transcode(docselem->getAttribute(X("format")));
							doctype->fileFormat(p);
							XMLString::release(&p);
							
							p=XMLString::transcode(docselem->getAttribute(X("dir")));
							doctype->docDirectory(p);
							XMLString::release(&p);
							
							p=XMLString::transcode(docselem->getAttribute(X("capacity")));
							doctype->capacity(p);
							XMLString::release(&p);
							
							readConfigIndexes(document, docselem, *doctype);
							
							doctype->load();
							
							m_doctypes.insert(make_pair(doctype->docDirectory(),doctype));
						}
						
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
		
		// TODO: make cross-referencing connections between the index groups
	}
	
	Ouzo::~Ouzo()
	{
	}
	
	void Ouzo::addDocument(bfs::path docfile)
	{
		DocumentBase& doctype=findDocType(docfile);
		doctype.addDocument(docfile);
	}
	
	void Ouzo::delDocument(bfs::path docfile)
	{
		DocumentBase& doctype=findDocType(docfile);
		doctype.delDocument(docfile);
	}
	
	DocumentBase& Ouzo::findDocType(const bfs::path& docfile)
	{
		// If docfile is an absolute path, verify that it is in the config's documents/dir
		if (!docfile.has_root_path())
			throw Exception(__FILE__,__LINE__);

		// Find the DocumentBase that refers to documents in the directory docfile is in.
		std::map<bfs::path,DocumentBase*>::iterator itr=m_doctypes.find(docfile.parent_path());
		if (itr==m_doctypes.end())
		{
			throw Exception(__FILE__,__LINE__);
		}

		return *(itr->second);
	}

	std::ostream& operator<<(std::ostream& os, const Ouzo& ouzo)
	{
		os << "Config file     :" << ouzo.m_config_file << std::endl;
		
		std::map<bfs::path,DocumentBase*>::const_iterator itr_end=ouzo.m_doctypes.end();
		for (std::map<bfs::path,DocumentBase*>::const_iterator itr=ouzo.m_doctypes.begin(); itr!=itr_end; ++itr)
		{
			os << "======================" << endl
			   << " Documents:" << endl
			   << "======================" << endl
			   << *(itr->second) << endl;
		}
		
		return os;
	}
	
}

