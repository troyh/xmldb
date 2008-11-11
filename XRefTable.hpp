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

		typedef std::map<const Index*, Key> columns_type;
		typedef std::map<docid_t,columns_type> table_type;
		typedef table_type::size_type size_type;

		DocumentBase* m_pDB;
		table_type m_table;
		
	public:
		XRefTable(DocumentBase* pDB) : m_pDB(pDB) {}
		~XRefTable() {}

		void addColumn(const Index* idx);

		void putCell(docid_t row, const Key& key);
		Key getCell(docid_t row, Index* col) const;
	};
}

#endif
