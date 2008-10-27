#ifndef _OUZO_DOCUMENTBASE_HPP
#define _OUZO_DOCUMENTBASE_HPP

#include <iostream>
#include <vector>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>

namespace Ouzo
{
	namespace bfs=boost::filesystem;
	using namespace std;
	
	typedef uint32_t docid_t;
	
	class Index;
	
	class DocumentBase
	{
		friend ostream& operator<<(ostream& os, const DocumentBase& doctype);
	public:
		typedef enum { UNKNOWN=0, XML=1 } fileformat_type;
	private:
		fileformat_type m_fileformat;
		uint32_t m_capacity;
		bfs::path m_docdir;
		bfs::path m_datadir;

		std::map<bfs::path,docid_t> m_docidmap;
		boost::dynamic_bitset<> m_avail_docids;
		std::vector<Index*> m_indexes;

		void addXMLDocument(bfs::path fname, docid_t docid);
		void persist();
	public:	
		DocumentBase() : m_fileformat(UNKNOWN), m_capacity(0) {}
		~DocumentBase() {}

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
	};

	ostream& operator<<(ostream& os, const DocumentBase& doctype);

}

#endif
