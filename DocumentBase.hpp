#ifndef _OUZO_DOCUMENTTYPE_HPP
#define _OUZO_DOCUMENTTYPE_HPP

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
	
	class DocumentType
	{
		friend ostream& operator<<(ostream& os, const DocumentType& doctype);
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
		DocumentType() : m_fileformat(UNKNOWN), m_capacity(0) {}
		~DocumentType() {}

		void docDirectory(const char* dir) { m_docdir=dir; }
		void docDirectory(bfs::path dir) { m_docdir=dir; }
		bfs::path docDirectory() const { return m_docdir; }

		void dataDirectory(const char* dir) { m_datadir=dir; }
		void dataDirectory(bfs::path dir) { m_datadir=dir; }
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

	ostream& operator<<(ostream& os, const DocumentType& doctype);

}

#endif
