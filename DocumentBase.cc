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
#include <boost/timer.hpp>

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
				
				std::map<bfs::path,docid_t>::const_iterator itr_end=m_docidmap.end();
				for (std::map<bfs::path,docid_t>::const_iterator itr=m_docidmap.begin(); itr!=itr_end; ++itr)
				{
					m_docidmap_reverse[itr->second]=itr->first;
				}
				// TODO: load m_docidmap_reverse from file to get rid of the above loop
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
		// TODO: persist these into the same file
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
			// TODO: save m_docidmap_reverse to file
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
		m_docidmap_reverse[docid]=fname;
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
			m_docidmap_reverse.erase(docid);
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
		XRefTable* xref_tbl=getXRefTable();
		
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

					// Add document to x-ref table
					xref_tbl->putCell(docid,val);

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
	
	void DocumentBase::query(const Query::TermNode& q, Query::Results& results)
	{
		string idxname=q.key();
		Index* idx=getIndex(idxname);
		idx->load();

		boost::timer t;
		if (q.eqop()==Query::TermNode::eq || q.eqop()==Query::TermNode::ne || q.eqop()==Query::TermNode::lt || q.eqop()==Query::TermNode::gte)
		{
			const DocSet& ds=idx->get(q.val().c_str());
			results=ds;
		}

		if (q.eqop()==Query::TermNode::ne)
		{
			results.flip();
		}
		else if (q.eqop()==Query::TermNode::lt || q.eqop()==Query::TermNode::gt || q.eqop()==Query::TermNode::lte || q.eqop()==Query::TermNode::gte)
		{
			Query::Results gt_results(results.docbase());

			switch (Index::getType(*idx))
			{
				case Index::INDEX_TYPE_UNKNOWN:
					break;
				case Index::INDEX_TYPE_STRING : 
				{
					StringIndex* sidx=dynamic_cast<StringIndex*>(idx);
					StringIndex::const_iterator_type itr=sidx->lower_bound(q.val());
					StringIndex::const_iterator_type itr_end=sidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_UINT8  : 
				{
					UIntIndex<uint8_t>* uiidx=dynamic_cast< UIntIndex<uint8_t>* >(idx);
					UIntIndex<uint8_t>::const_iterator_type itr=uiidx->lower_bound(q.val());
					UIntIndex<uint8_t>::const_iterator_type itr_end=uiidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_UINT16 : 
				{
					UIntIndex<uint16_t>* uiidx=dynamic_cast< UIntIndex<uint16_t>* >(idx);
					UIntIndex<uint16_t>::const_iterator_type itr=uiidx->lower_bound(q.val());
					UIntIndex<uint16_t>::const_iterator_type itr_end=uiidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_UINT32 : 
				{
					UIntIndex<uint32_t>* uiidx=dynamic_cast< UIntIndex<uint32_t>* >(idx);
					UIntIndex<uint32_t>::const_iterator_type itr=uiidx->lower_bound(q.val());
					UIntIndex<uint32_t>::const_iterator_type itr_end=uiidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_FLOAT  :
				{
					FloatIndex* fidx=dynamic_cast<FloatIndex*>(idx);
					FloatIndex::const_iterator_type itr=fidx->lower_bound(q.val());
					FloatIndex::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_DATE   :
				{
					DateIndex* fidx=dynamic_cast<DateIndex*>(idx);
					DateIndex::const_iterator_type itr=fidx->lower_bound(q.val());
					DateIndex::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_TIME   :
				{
					TimeIndex* fidx=dynamic_cast<TimeIndex*>(idx);
					TimeIndex::const_iterator_type itr=fidx->lower_bound(q.val());
					TimeIndex::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_SINT8  : 
				{
					UIntIndex<int8_t>* fidx=dynamic_cast< UIntIndex<int8_t>* >(idx);
					UIntIndex<int8_t>::const_iterator_type itr=fidx->lower_bound(q.val());
					UIntIndex<int8_t>::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_SINT16 : 
				{
					UIntIndex<int16_t>* fidx=dynamic_cast< UIntIndex<int16_t>* >(idx);
					UIntIndex<int16_t>::const_iterator_type itr=fidx->lower_bound(q.val());
					UIntIndex<int16_t>::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				case Index::INDEX_TYPE_SINT32 :
				{
					UIntIndex<int32_t>* fidx=dynamic_cast< UIntIndex<int32_t>* >(idx);
					UIntIndex<int32_t>::const_iterator_type itr=fidx->lower_bound(q.val());
					UIntIndex<int32_t>::const_iterator_type itr_end=fidx->end();
					for (itr++; itr!=itr_end; itr++)
					{
						gt_results|=itr->second;
					}
					break;
				}
				
			}

			if (q.eqop()==Query::TermNode::gt)
			{
				// Do nothing
			}
			else if (q.eqop()==Query::TermNode::gte)
			{
				results|=gt_results;
			}
			else if (q.eqop()==Query::TermNode::lte)
			{
				results=gt_results;
				results.flip();
			}
			else if (q.eqop()==Query::TermNode::lt)
			{
				results|=gt_results;
				results.flip();
			}
			
		}
		results.queryTime(t.elapsed());
		
	}
	
	void DocumentBase::getDocFilenames(const Query::Results& results, std::vector<bfs::path>& docs)
	{
		for (DocSet::size_type n=results.find_first(); n!=DocSet::npos; n=results.find_next(n))
		{
			docs.push_back(m_docidmap_reverse[n]);
		}
	}

	std::ostream& operator<<(std::ostream& os, const DocumentBase& doctype)
	{
		dynamic_bitset<>::size_type nextdocid=doctype.m_avail_docids.find_first()+1;
		
		os << "Doc dir         :" << doctype.docDirectory() << std::endl
		   << "Data dir        :" << doctype.dataDirectory() << std::endl
		   << "Doc capacity    :" << doctype.capacity() << std::endl
		   << "Next avail docid:" << nextdocid << std::endl;
		
		if (doctype.m_XRefs)
		{
			os << "X-Reference table: " << *doctype.m_XRefs << std::endl;
		}
		
		os << "-------------------------" << std::endl
		   << " Doc-ID map: " << std::endl
		   << "-------------------------" << std::endl;

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
		idx->setDocBase(this);
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
	
	XRefTable* DocumentBase::getXRefTable()
	{
		if (!m_XRefs)
			m_XRefs=new XRefTable(this);
		return m_XRefs;
	}

}
