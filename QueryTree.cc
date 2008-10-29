#include "QueryTree.hpp"
#include "DocumentBase.hpp"

namespace Ouzo
{
	namespace Query
	{
		
		Results::Results(DocumentBase* pDB)
			: DocSet(pDB->capacity()), m_pDB(pDB) 
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
		
		void Results::convertToDocBase(Results& model)
		{
			if (model.m_pDB==m_pDB)
			{
				// No need to do anything, they're already the same
				return;
			}
			
			// TODO: implement this
			// Find the way to convert from this docbase to model's docbase
			
		}
		
	}
}
