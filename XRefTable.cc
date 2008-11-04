#include "XRefTable.hpp"
#include "DocumentBase.hpp"

namespace Ouzo
{
	
void XRefTable::addColumn(const Index* idx)
{
	if (m_table[0].find(idx->getID())==m_table[0].end()) // Avoid adding duplicates
		m_table[0][idx->getID()]=0; // 0 is a dummy docid
}

void XRefTable::putCell(docid_t row, const char* val)
{
	// Iterate the indexes that are represented in the columns, get their lookupid and store it in the cell
	columns_type::iterator itr_end=m_table[row].end();
	for (columns_type::iterator itr=m_table[row].begin(); itr!=itr_end; ++itr)
	{
		Index* idx=Index::getIndexFromID(itr->first);
		Index::lookupid_t lookupid=idx->getLookupID(val);
		
		m_table[row][itr->first]=lookupid;
	}
}

Index::lookupid_t XRefTable::getCell(docid_t row, Index::indexid_t col) const
{
	table_type::const_iterator itr=m_table.find(row);
	if (itr==m_table.end())
		return Index::lookupid_t_nil;
	if (itr->second.find(col)==itr->second.end())
		return Index::lookupid_t_nil;
	return itr->second.find(col)->second;
}

ostream& operator<<(ostream& os, const XRefTable& tbl)
{
	os << " docid | ";
	XRefTable::table_type& tt=(XRefTable::table_type&)tbl.m_table;
	std::map<Index::indexid_t, docid_t>::iterator itr_end=tt[0].end();
	for (std::map<Index::indexid_t, docid_t>::iterator itr=tt[0].begin(); itr!=itr_end; ++itr)
	{
		Index* pIdx=Index::getIndexFromID(itr->first);
		os << pIdx->getDocBase()->name() << '.' << pIdx->name() << " | ";
	}
	os << std::endl;
	return os;
}


}
