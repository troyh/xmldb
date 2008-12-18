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
#include "QueryTree.hpp"

namespace Ouzo
{
	namespace bfs=boost::filesystem;
	using namespace std;
	using namespace boost;
	using namespace xercesc;

	class Index;
	
	// typedef key_t* (*keyfactory_func)(Index*);
	// 
	// key_t* stdkey(Index*);
	// key_t* stringkey(Index*);
	
	class Ouzo
	{
		friend ostream& operator<<(ostream& os, const Ouzo& ouzo);

		map<bfs::path,DocumentBase*> m_doctypes;
		bfs::path m_config_file;
		// static std::map<std::string,keyfactory_func> s_keyfactories;

		DocumentBase& findDocType(const bfs::path& docfile);
		// const char* getNodeValue(const DOMNode* node,const char* tag);
		void getValues(DOMDocument* document,const char*);
		void readConfigIndexes(DOMDocument* document, const DOMElement* node, DocumentBase& doctype);
		void convertToDocBase(Query::Results& from, DocumentBase* pDB) const;
		
	public:	
		
		// static key_t* createKey(Index*);
		static Index* createIndex(Index::key_t::key_type kt, const char* name, const char* keyspec, uint32_t capacity);
		
		Ouzo(bfs::path config_file);
		~Ouzo();
		
		void addDocument(std::vector<bfs::path> docfiles, void (*func)(void*)=0, void* voidp=0);
		void delDocument(bfs::path docfile);
		
		DocumentBase* getDocBase(std::string name) const;
	
		void fetch(const Query::Node& q, Query::Results& results) const;
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
