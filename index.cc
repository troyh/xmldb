#include <iostream>
#include <map>
#include <vector>
#include <fstream>
// #include <sstream>
// #include <xqilla/xqilla-simple.hpp>
#include <boost/filesystem.hpp>
// #include "XMLDoc.hpp"
#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
// #include <xercesc/parsers/XercesDOMParser.hpp>
#include <xqilla/xqilla-dom3.hpp>
#include <boost/progress.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;
using namespace xercesc;
using namespace boost::filesystem;

const char* getNodeValue(const DOMNode* node,const char* tag)
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

int main(int argc, char *argv[]) {
	
	path idxfile=(argv[1]);
	
	boost::progress_timer timer;
	
	// Initialise Xerces-C and XQilla using XQillaPlatformUtils
	XQillaPlatformUtils::initialize();

	{
	// Get the XQilla DOMImplementation object
	DOMImplementation *xqillaImplementation=DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

	// Create a DOMBuilder object
	DOMBuilder *builder = xqillaImplementation->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
	builder->setFeature(X("namespaces"), true);
	builder->setFeature(X("http://apache.org/xml/features/validation/schema"), true);
	builder->setFeature(X("validation"), true);

	// Parse a DOMDocument
	StdInInputSource* ins=new StdInInputSource;
	Wrapper4InputSource wis(ins);
	DOMDocument *document = builder->parse(wis);
	if(document == 0) {
	        std::cerr << "Document not found." << std::endl;
	        return 1;
	}

	try {
	        // Parse an XPath 2 expression
	        const DOMXPathExpression *expression = document->createExpression(X("//entries"), 0);

	        // Execute the query
	        XPath2Result *result = (XPath2Result*)expression->evaluate(document, XPath2Result::ITERATOR_RESULT, 0);

	        // Create a DOMWriter to output the nodes
	        // DOMWriter *writer = xqillaImplementation->createDOMWriter();
	        // StdOutFormatTarget target;

	        // Iterate over the results, printing them
			map< string, vector<uint32_t> > idx;
	        while(result->iterateNext()) {
                // writer->writeNode(&target, *(result->asNode()));
                // std::cout << std::endl;
				for (DOMNode* kid=result->asNode()->getFirstChild();kid;kid=kid->getNextSibling())
				{
					if (XMLString::equals(kid->getNodeName(),X("entry")))
					{
						string key=getNodeValue(kid,"key");
						path file=getNodeValue(kid,"file");
						string fname=boost::filesystem::basename(file);
						uint32_t docno=strtol(fname.c_str(),NULL,10);
						
						// cout << key << ':' << file << endl;
						
						map< string, vector<uint32_t> >::iterator itr=idx.find(key);
						if (itr==idx.end()) // Doesn't exist
						{
							vector<uint32_t> docs(1,docno);
							idx.insert(make_pair(key,docs));
						}
						else // Exists, add file to vector
						{
							vector<uint32_t>& docs=idx[key];
							docs.push_back(docno);
						}
					}
				}
	        }

	        // Clean up all the objects we have created
	        // writer->release();
	        result->release();
	        ((XQillaExpression*)expression)->release();
	
			std::ofstream ofs(idxfile.string().c_str());
			try
			{
				boost::archive::text_oarchive ar(ofs);
				boost::serialization::save(ar,idx,0);
				
				cout << "Indexed " << idx.size() << " items" << endl;
				for (map< string, vector<uint32_t> >::iterator itr=idx.begin(); itr!=idx.end(); ++itr)
				{
					cout << itr->first << ':';
					vector<uint32_t>& items=itr->second;
					for (size_t i=0;i < items.size();++i)
					{
						cout << items[i] << ',';
					}
					cout << endl;
				}
			}
			catch (boost::archive::archive_exception& x)
			{
				cout << "Archive exception: " << x.what() << endl;
			}
			

	}
	catch(XQillaException &e) {
	        std::cerr << "XQillaException: " << UTF8(e.getString()) << std::endl;
	        return 1;
	}

	builder->release();
	}

	// Terminate Xerces-C and XQilla using XQillaPlatformUtils
	XQillaPlatformUtils::terminate();

	return 0;
}
