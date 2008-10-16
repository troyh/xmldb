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

namespace Ouzo
{
	typedef uint32_t docid_t;

	namespace bfs=boost::filesystem;
	using namespace std;
	using namespace boost;
	using namespace xercesc;

	class Index;
	
	class Ouzo
	{
		friend ostream& operator<<(ostream& os, const Ouzo& ouzo);
	public:
		typedef enum { XML } doctype; // Supported document types
	private:
		std::map<bfs::path,docid_t> m_docidmap;
		dynamic_bitset<> m_avail_docids;
		std::vector<Index*> m_indexes;
		bfs::path m_config_file;
		Config m_cfg;

		void addXMLDocument(bfs::path fname, docid_t docid);
		const char* getNodeValue(const DOMNode* node,const char* tag);
		void getValues(DOMDocument* document,const char*);
		void persist();
		
	public:	
		Ouzo(bfs::path config_file);
		~Ouzo();
		
		Config config() const { return m_cfg; }
	
		void addDocument(bfs::path docfile,doctype type=XML);
		void delDocument(bfs::path docfile);
		
		Index* getIndex(const bfs::path& fname);
	
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
