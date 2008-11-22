#include "Ouzo.hpp"
#include "Index.hpp"

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


// TODO: For unique indexes, make DocSet efficient, i.e., store only one docid, not a bitset or vector
// TODO: support queries
// TODO: make it thread-safe
// TODO: In DocSet, don't always allocate a vector and a dynamic_bitset, only the one you need



namespace Ouzo
{
	// std::map<std::string,keyfactory_func> Ouzo::s_keyfactories;

	using namespace xercesc;
	
	// key_t* stdkey(Index* idx)
	// {
	// 	if (!strcasecmp(idx->keyType(),"uint8" )) return new Key((uint8_t)0);
	// 	if (!strcasecmp(idx->keyType(),"uint16")) return new Key((uint16_t)0);
	// 	if (!strcasecmp(idx->keyType(),"uint32")) return new Key((uint32_t)0);
	// 	if (!strcasecmp(idx->keyType(),"uint64")) return new Key((uint64_t)0);
	// 	if (!strcasecmp(idx->keyType(),"int8"  )) return new Key((int8_t)0);
	// 	if (!strcasecmp(idx->keyType(),"int16" )) return new Key((int16_t)0);
	// 	if (!strcasecmp(idx->keyType(),"int32" )) return new Key((int32_t)0);
	// 	if (!strcasecmp(idx->keyType(),"int64" )) return new Key((int64_t)0);
	// 	if (!strcasecmp(idx->keyType(),"double")) return new Key((double)0);
	// 	if (!strcasecmp(idx->keyType(),"char8" )) return new Key((const char*)"        ");
	// 	
	// 	return new Key();
	// }
	// 
	// Key* stringkey(Index* idx)
	// {
	// 	return new StringKey();
	// }
	// 
	// Key* Ouzo::createKey(Index* idx)
	// {
	// 	std::map<std::string,keyfactory_func>::const_iterator itr=s_keyfactories.find(idx->keyType());
	// 	if (itr==s_keyfactories.end())
	// 		throw Exception(__FILE__,__LINE__);
	// 	return itr->second(idx);
	// }
	
	Index* Ouzo::createIndex(Index::key_t::key_type kt, const char* name, const char* keyspec, uint32_t capacity)
	{
		switch (kt)
		{
			case Index::key_t::KEY_TYPE_INT8   :
			case Index::key_t::KEY_TYPE_INT16  :
			case Index::key_t::KEY_TYPE_INT32  :
			case Index::key_t::KEY_TYPE_INT64  :
			case Index::key_t::KEY_TYPE_UINT8  :
			case Index::key_t::KEY_TYPE_UINT16 :
			case Index::key_t::KEY_TYPE_UINT32 :
			case Index::key_t::KEY_TYPE_UINT64 :
			case Index::key_t::KEY_TYPE_DBL    :
			case Index::key_t::KEY_TYPE_CHAR8  :
			case Index::key_t::KEY_TYPE_DATE :
			case Index::key_t::KEY_TYPE_TIME :
			case Index::key_t::KEY_TYPE_FLOAT:
				return new Index(name,kt,keyspec,capacity);
				break;
			case Index::key_t::KEY_TYPE_STRING :
				return new StringIndex(name,keyspec,capacity);
				break;
			default:
				return NULL;
				break;
		}
	}
	

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
					
					char* idxtype_s=XMLString::transcode(idxtype);
					
					char* idxkey_s=getXPathVal("./key",document,pElem);
					if (idxkey_s)
					{
						char* idxname_s=XMLString::transcode(idxname);
						
						Index* p=Ouzo::createIndex(Index::key_t::getKeyType(idxtype_s), idxname_s, idxkey_s, doctype.capacity());
						
						bfs::path idxpath(doctype.dataDirectory());
						idxpath /= idxname_s;
						p->setFilename(idxpath);
						if (!bfs::exists(p->filename()))
						{
							p->initFile();
						}
						
						doctype.addIndex(p);
						
						XMLString::release(&idxtype_s);
						XMLString::release(&idxname_s);
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
		// Create the standard key types
		// s_keyfactories.clear();
		// s_keyfactories.insert(make_pair("string",stringkey));
		// s_keyfactories.insert(make_pair("uint8" ,stdkey));
		// s_keyfactories.insert(make_pair("uint16",stdkey));
		// s_keyfactories.insert(make_pair("uint32",stdkey));
		// s_keyfactories.insert(make_pair("uint64",stdkey));
		// s_keyfactories.insert(make_pair("int8"  ,stdkey));
		// s_keyfactories.insert(make_pair("int16" ,stdkey));
		// s_keyfactories.insert(make_pair("int32" ,stdkey));
		// s_keyfactories.insert(make_pair("int64" ,stdkey));
		// s_keyfactories.insert(make_pair("double",stdkey));
		// s_keyfactories.insert(make_pair("char8" ,stdkey));

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

							char* p=XMLString::transcode(docselem->getAttribute(X("name")));
							DocumentBase* doctype=new DocumentBase(p);
							XMLString::release(&p);

							p=XMLString::transcode(docselem->getAttribute(X("format")));
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
		
		////////////////////////////////////////////////////////////////
		// Make cross-referencing connections between the index groups
		////////////////////////////////////////////////////////////////
		
		// Iterate the indexes in each DocBase collecting Indexes by name, the name is
		// the key for cross-referencing between DocBases
		std::map< std::string, std::vector<const Index*> > xrefs;

		map<bfs::path,DocumentBase*>::const_iterator itr=m_doctypes.begin();
		map<bfs::path,DocumentBase*>::const_iterator itr_end=m_doctypes.end();
		for (; itr!=itr_end; ++itr)
		{
			DocumentBase* pDB=itr->second;
			for (size_t i=0;i < pDB->indexCount(); ++i) // Iterate indexes
			{
				const Index* idx=pDB->getIndex(i);
				xrefs[idx->name()].push_back(idx);
			}
		}
		
		// {
		// 	std::map< std::string, std::vector<const Index*> >::const_iterator itr_end=xrefs.end();
		// 	for (std::map< std::string, std::vector<const Index*> >::const_iterator itr=xrefs.begin(); itr!=itr_end; ++itr)
		// 	{
		// 		std::map< std::string, std::vector<Index*> >::size_type n=itr->second.size();
		// 		if (n > 1)
		// 		{
		// 			for (std::map< std::string, std::vector<Index*> >::size_type i=0;i < n; ++i)
		// 			{
		// 				DocumentBase* db=itr->second[i]->getDocBase();
		// 				XRefTable* tbl=db->getXRefTable();
		// 				tbl->addColumn(itr->second[i]);
		// 			}
		// 		}
		// 	}
		// }


		// Iterate the DocBases again, if any index is referenced more than once in xrefs, make an xref table for it
		for (itr=m_doctypes.begin(); itr!=itr_end; ++itr)
		{
			DocumentBase* pDB=itr->second;
			
			// Iterate the indexes and see if any of them are referenced again elsewhere
			for (size_t i=0;i < pDB->indexCount(); ++i)
			{
				const Index* idx=pDB->getIndex(i);
				std::map< std::string, std::vector<Index*> >::size_type n=xrefs[idx->name()].size();
				if (n > 1)
				{
					XRefTable* tbl=pDB->getXRefTable();
		
					for (std::map< std::string, std::vector<Index*> >::size_type i=0;i < n; ++i)
					{
						const Index* pIdx=xrefs[idx->name()][i];
						if (pIdx!=idx)
							tbl->addColumn(pIdx);
					}
				}
				
			}
		}
		
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

	DocumentBase* Ouzo::getDocBase(std::string name) const
	{
		std::map<bfs::path,DocumentBase*>::const_iterator itr_end=m_doctypes.end();
		for (std::map<bfs::path,DocumentBase*>::const_iterator itr=m_doctypes.begin(); itr!=itr_end; ++itr)
		{
			if (itr->second->name()==name)
				return itr->second;
		}
		return NULL;
	}
	
	void Ouzo::convertToDocBase(Query::Results& from, DocumentBase* pDBTo) const
	{
		DocumentBase* pDBFrom=from.docbase();
		
		if (pDBFrom==pDBTo)
		{
			// No need to do anything, they're already the same
			return;
		}
		
		// Get an Index that we can use to cross-reference
		// Index* pIdx=findXRef(pDBFrom,pDBTo);
		// if (pIdx)
		// {
		// 	// pIdx points to an Index whose keys correspond to docids that we have in ourselves (we're a DocSet)
		// 	
		// 	// Create a new DocSet to put the results in 
		// 	Query::Results* newresults=new Query::Results(pDBTo);
		// 	
		// 	// Iterate my result documents, find the same docid in pIdx and combine the docset
		// 	for (DocSet::size_type n=from.find_first(); n!=DocSet::npos; n=from.find_next(n))
		// 	{
		// 		docid_t docid=n+1; // +1 correct?
		// 		pDBFrom->getDocBase();
		// 		(*newresults)|=pIdx->get(key); // Combine them
		// 	}
		// 	
		// 	// Convert ourselves to the newresults
		// 	*this=newresults;
		// }
			
	}
	
	void Ouzo::fetch(const Query::Node& q, Query::Results& results) const
	{
		const Query::TermNode* termnode=dynamic_cast<const Query::TermNode*>(&q);
		const Query::BooleanNode* boolnode=dynamic_cast<const Query::BooleanNode*>(&q);
		
		if (termnode)
		{
			std::string docbasename=q.getDocBaseName();
			DocumentBase* pDB=getDocBase(docbasename);
			pDB->load();
			pDB->query(*termnode, results);
		}
		else if (boolnode)
		{
			Query::Node* left=boolnode->left();
			Query::Node* right=boolnode->right();
			
			if (!left)
				throw Exception(__FILE__, __LINE__);
			if (!right)
				throw Exception(__FILE__, __LINE__);

			DocumentBase* leftdb=getDocBase(left->getDocBaseName());
			DocumentBase* rightdb=getDocBase(right->getDocBaseName());

			if (!leftdb)
				throw Exception(__FILE__, __LINE__);
			if (!rightdb)
				throw Exception(__FILE__, __LINE__);
			
			Query::Results l_results(leftdb);
			Query::Results r_results(rightdb);

			fetch(*left, l_results);
			fetch(*right, r_results);
			
			// convert r_results to the type we need (if not already)
			convertToDocBase(r_results,results.docbase());
			
			// convert l_results to the type we need (if not already)
			convertToDocBase(l_results,results.docbase());

			switch(boolnode->oper())
			{
				case Query::BooleanNode::OR:
					results=l_results;
					results|=r_results;
					break;
				case Query::BooleanNode::AND:
					results=l_results;
					results&=r_results;
					break;
				case Query::BooleanNode::UNDEF:
					throw Exception(__FILE__,__LINE__);
					break;
			}
			
			if (boolnode->isUnaryNot())
			{
				results.flip();
			}
		}
		else
			throw Exception(__FILE__,__LINE__); // Unknown type
		
	}
	

	std::ostream& operator<<(std::ostream& os, const Ouzo& ouzo)
	{
		os << "Config file     :" << ouzo.m_config_file << std::endl;
		
		std::map<bfs::path,DocumentBase*>::const_iterator itr_end=ouzo.m_doctypes.end();
		for (std::map<bfs::path,DocumentBase*>::const_iterator itr=ouzo.m_doctypes.begin(); itr!=itr_end; ++itr)
		{
			os << "======================" << endl
			   << " DocumentBase: " << itr->second->name() << endl
			   << "======================" << endl
			   << *(itr->second) << endl;
		}
		
		return os;
	}

}
