#ifndef _OUZO_XREFTABLE_HPP
#define _OUZO_XREFTABLE_HPP

#include <map>

#include "Index.hpp"

namespace Ouzo
{
	class DocumentBase;
	
	class XRefTable
	{
		friend ostream& operator<<(ostream& os, const XRefTable& tbl);
	public:
		typedef std::map<Index::indexid_t, Index::lookupid_t> columns_type;
		typedef std::map< docid_t, columns_type > table_type;
		typedef table_type::size_type size_type;
	private:
		DocumentBase* m_pDB;
		table_type m_table;
	public:
		XRefTable(DocumentBase* pDB) : m_pDB(pDB) {}
		~XRefTable() {}

		void addColumn(const Index* idx);

		void putCell(docid_t row, const char* val);
		Index::lookupid_t getCell(docid_t row, Index::indexid_t col) const;
	};
}

#endif
