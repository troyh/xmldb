#include "XRefTable.hpp"
#include "DocumentBase.hpp"

namespace Ouzo
{
	
void XRefTable::addColumn(const Index* idx)
{
	Index::key_t* k=idx->createKey();
	if (m_table[0].find(idx)==m_table[0].end()) // Avoid adding duplicates
		m_table[0][idx]=*k; // k is a dummy key
	delete k;
}

void XRefTable::putCell(docid_t row, const Index::key_t& key)
{
	// Iterate the indexes that are represented in the columns, get their lookupid and store it in the cell
	columns_type::iterator itr_end=m_table[row].end();
	for (columns_type::iterator itr=m_table[row].begin(); itr!=itr_end; ++itr)
	{
		const Index* idx=itr->first;
		m_table[row][idx]=key;
	}
}

Index::key_t XRefTable::getCell(docid_t row, Index* col) const
{
	table_type::const_iterator itr=m_table.find(row);
	if (itr==m_table.end())
		return Index::key_t(col);
	columns_type::const_iterator itr2=itr->second.find(col);
	if (itr2==itr->second.end())
		return Index::key_t(col);
	return Index::key_t(itr2->second);
}

ostream& operator<<(ostream& os, const XRefTable& tbl)
{
	os << " docid | ";
	XRefTable::table_type& tt=(XRefTable::table_type&)tbl.m_table;
	XRefTable::columns_type::iterator itr_end=tt[0].end();
	for (XRefTable::columns_type::iterator itr=tt[0].begin(); itr!=itr_end; ++itr)
	{
		const Index* pIdx=itr->first;
		os << pIdx->getDocBase()->name() << '.' << pIdx->name() << " | ";
	}
	os << std::endl;
	return os;
}


}
