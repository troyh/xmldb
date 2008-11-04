#include "QueryTree.hpp"
#include "DocumentBase.hpp"

namespace Ouzo
{
	namespace Query
	{
		
		Results::Results(DocumentBase* pDB)
			: DocSet(pDB->capacity()), m_pDB(pDB), m_timer(0) 
		{
		}
		
		Results::Results(const Results& r)
			: DocSet(r)
		{
			m_pDB=r.m_pDB;
		}
		
		Results& Results::operator=(const DocSet& ds)
		{
			DocSet::operator=(ds);
			return *this;
		}
		
		Results& Results::operator=(const Results& r)
		{
			DocSet::operator=(r);
			return *this;
		}
		
	
	}
}
