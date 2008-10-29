#ifndef _OUZO_DOCUMENTBASE_HPP
#define _OUZO_DOCUMENTBASE_HPP

#include <iostream>
#include <vector>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>

#include "Index.hpp"
#include "QueryTree.hpp"

namespace Ouzo
{
	namespace bfs=boost::filesystem;
	using namespace std;
	
	class Index;
	
	class DocumentBase
	{
		friend ostream& operator<<(ostream& os, const DocumentBase& doctype);
	public:
		typedef enum { UNKNOWN=0, XML=1 } fileformat_type;
	private:
		std::string m_name;
		fileformat_type m_fileformat;
		uint32_t m_capacity;
		bfs::path m_docdir;
		bfs::path m_datadir;

		std::map<bfs::path,docid_t> m_docidmap;
		std::map<docid_t,bfs::path> m_docidmap_reverse;
		boost::dynamic_bitset<> m_avail_docids;
		std::vector<Index*> m_indexes;

		void addXMLDocument(bfs::path fname, docid_t docid);
		void persist();
		
		// Prevent these from being used:
		DocumentBase(const DocumentBase&);
		DocumentBase& operator=(const DocumentBase&);
	public:	
		DocumentBase(std::string name) : m_name(name), m_fileformat(UNKNOWN), m_capacity(0) {}
		~DocumentBase() {}
		
		std::string name() const { return  m_name; }

		void docDirectory(const char* dir);
		void docDirectory(bfs::path dir);
		bfs::path docDirectory() const { return m_docdir; }

		void dataDirectory(const char* dir);
		void dataDirectory(bfs::path dir);
		bfs::path dataDirectory() const { return m_datadir; }
		
		void capacity(const char* str);
		void capacity(uint32_t n) { m_capacity=n; }
		uint32_t capacity() const { return m_capacity; }
		
		void fileFormat(const char*);
		
		void addIndex(Index*);
		
		// Index* getIndex(uint32_t n);

		void addDocument(bfs::path docfile);
		void delDocument(bfs::path docfile);
		
		void load();
		
		Index* getIndex(std::string name);
		
		void query(const Query::TermNode& q, Query::Results& results);
		
		void getDocFilenames(const Query::Results& results, std::vector<bfs::path>& docs);
		
		
	};

	ostream& operator<<(ostream& os, const DocumentBase& doctype);

}

#endif
