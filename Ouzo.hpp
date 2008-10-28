#ifndef _OUZO_OUZO_HPP
#define _OUZO_OUZO_HPP

#include <execinfo.h>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/shared_ptr.hpp>

#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocument.hpp>

#include "Config.hpp"
#include "DocumentBase.hpp"

namespace Ouzo
{
	namespace bfs=boost::filesystem;
	using namespace std;
	using namespace boost;
	using namespace xercesc;

	class Index;
	
	class Ouzo
	{
		friend ostream& operator<<(ostream& os, const Ouzo& ouzo);

		map<bfs::path,DocumentBase*> m_doctypes;
		bfs::path m_config_file;

		DocumentBase& findDocType(const bfs::path& docfile);
		// const char* getNodeValue(const DOMNode* node,const char* tag);
		void getValues(DOMDocument* document,const char*);
		void readConfigIndexes(DOMDocument* document, const DOMElement* node, DocumentBase& doctype);
		
	public:	
		Ouzo(bfs::path config_file);
		~Ouzo();
		
		void addDocument(bfs::path docfile);
		void delDocument(bfs::path docfile);
		
		DocumentBase* getDocBase(std::string name);
	
		// Results fetch(const Query& q) const;
	};
	
	ostream& operator<<(ostream& os, const Ouzo& ouzo);
	
}

#include "Index.hpp"
#include "BitmapAllocator.hpp"
#include "DocSet.hpp"
#include "Exception.hpp"
#include "Mutex.hpp"
#include "StringIndex.hpp"
#include "UIntIndex.hpp"
#include "XMLDoc.hpp"

#endif
