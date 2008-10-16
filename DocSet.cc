#include "Ouzo.hpp"

namespace Ouzo
{
	
DocSet::DocSet(size_t capacity)
	: m_type(bitmap), 
	m_docs_arr(new std::vector<docid_t>), 
	m_docs_bitmap(new bitset_type(capacity,0,BitmapAllocator<unsigned long>())),
	m_capacity(capacity)
{
}

DocSet::DocSet(DocSet& ds)
{
	*this=ds;
}

DocSet::DocSet(const DocSet& ds)
{
	*this=(DocSet&)ds;
}

DocSet& DocSet::operator=(DocSet& ds) 
{ 
	m_docs_bitmap=ds.m_docs_bitmap; 
	m_docs_arr=ds.m_docs_arr; 
	m_type=ds.m_type; 
	m_capacity=ds.m_capacity;
	return *this; 
}

DocSet& DocSet::operator=(const DocSet& ds) 
{ 
	return *this=(DocSet&)ds; 
}

size_t DocSet::size() const
{
	switch (m_type)
	{
		case arr:
			return m_docs_arr->size();
			break;
		case bitmap:
			return m_docs_bitmap->size();
			break;
		default:
			throw Exception(__FILE__,__LINE__);
			break;
	}
}

void DocSet::set(docid_t docno)
{
	switch (this->type())
	{
	case bitmap:
		m_docs_bitmap->set(docno);
		break;
	case arr:
	{
		// See if it's already there
		std::vector<docid_t>::const_iterator itr_end=m_docs_arr->end();
		bool found=false;
		for (std::vector<docid_t>::const_iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
		{
			if (*itr==docno)
				found=true;
		}
		if (!found) // only add it if it's not already there
			m_docs_arr->push_back(docno);
		break;
	}
	default:
		throw Exception(__FILE__,__LINE__);
		break;
	}
}

void DocSet::clr(docid_t docno)
{
	if (this->type()==bitmap)
	{
		m_docs_bitmap->reset(docno);
	}
	else // Array type
	{
		// Remove from vector
		std::vector<docid_t>::iterator itr_end=m_docs_arr->end();
		for (std::vector<docid_t>::iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
		{
			if (*itr==docno)
			{
				m_docs_arr->erase(itr);
				break;
			}
		}
	}
}

DocSet& DocSet::operator|=(const DocSet& ds)
{
	if (this->type()==bitmap && ds.type()==bitmap) // Both are bitmaps
	{
		*(m_docs_bitmap)|=*(ds.m_docs_bitmap);
	}
	else
	{
		// TODO: combine manually
	}
	
	return *this;
}

DocSet& DocSet::operator&=(const DocSet& ds)
{
	if (this->type()==bitmap && ds.type()==bitmap) // Both are bitmaps
	{
		*(m_docs_bitmap)&=*(ds.m_docs_bitmap);
	}
	else
	{
		// TODO: combine manually
	}
	
	return *this;
}

void DocSet::load(istream& is)
{
	is.read((char*)&m_type,sizeof(m_type));
	
	if (!is.good())
		throw Exception(__FILE__,__LINE__);
	
	switch (m_type)
	{
		case arr:
		{
			m_docs_arr->clear();

			std::vector<docid_t>::size_type n;
			is.read((char*)&n,sizeof(n));

			if (!is.good())
				throw Exception(__FILE__,__LINE__);
			
			for(size_t i = 0; i < n; ++i)
			{
				docid_t docid;
				is.read((char*)&docid,sizeof(docid));

				if (!is.good())
					throw Exception(__FILE__,__LINE__);
				
				m_docs_arr->push_back(docid);
			}

			break;
		}
		case bitmap:
		{
			BitmapAllocator<bitset_type::block_type>::size_type n;
			is.read((char*)&n,sizeof(n));
			
			if (!is.good())
				throw Exception(__FILE__,__LINE__);
			
			m_docs_bitmap->clear();
			m_docs_bitmap->resize(n);
			
			BitmapAllocator< bitset_type::block_type > pa=m_docs_bitmap->get_allocator();
			if (pa.sizeInBytes()<n)
				throw Exception(__FILE__,__LINE__);
				
			is.read(pa.startOfSpace(),n);
			
			if (!is.good())
				throw Exception(__FILE__,__LINE__);
			
			break;
		}
		default:
			break;
	}
	
	// Should we convert from one type of data structure to the other for efficiency?
	set_type besttype=mostEfficientType();
	if (besttype!=m_type)
	{
		convertToType(besttype);
	}
}

void DocSet::save(ostream& os) const
{
	os.write((char*)&m_type,sizeof(m_type));
	if (!os.good())
		throw Exception(__FILE__,__LINE__);

	switch (m_type)
	{
		case arr:
		{
			std::vector<docid_t>::size_type n=m_docs_arr->size();
			os.write((char*)&n,sizeof(n));
			
			for(std::vector<docid_t>::size_type i = 0; i < n; ++i)
			{
				docid_t docid=(*m_docs_arr)[i];
				os.write((char*)&docid,sizeof(docid));
			}
			break;
		}
		case bitmap:
		{
			BitmapAllocator< bitset_type::block_type > pa=m_docs_bitmap->get_allocator();
			BitmapAllocator< bitset_type::block_type >::size_type n=pa.sizeInBytes();
			
			os.write((char*)&n,sizeof(n));
			if (!os.good())
				throw Exception(__FILE__,__LINE__);
				
			os.write(pa.startOfSpace(),n);
			if (!os.good())
				throw Exception(__FILE__,__LINE__);
			break;
		}
		default:
			break;
	}
}

uint32_t DocSet::sizeInBytes() const
{
	std::vector<docid_t>::size_type arrn=m_docs_arr->size();
	
	BitmapAllocator< bitset_type::block_type > pa=m_docs_bitmap->get_allocator();
	BitmapAllocator< bitset_type::block_type >::size_type bitsn=pa.sizeInBytes();
	
	return max(sizeof(arrn)+arrn,sizeof(bitsn)+bitsn);
}


ostream& operator<<(ostream& os, const DocSet& ds)
{
	switch (ds.m_type)
	{
		case DocSet::arr:
		{
			bool first=true;
			for (std::vector<docid_t>::const_iterator itr=ds.m_docs_arr->begin(); itr!=ds.m_docs_arr->end(); ++itr)
			{
				if (!first)
					os << ',';
				os << *itr;
				first=false;
			}
			break;
		}
		case DocSet::bitmap:
		{
			bool first=true;
			for (DocSet::bitset_type::size_type n=ds.m_docs_bitmap->find_first(); n!=DocSet::bitset_type::npos; n=ds.m_docs_bitmap->find_next(n))
			{
				if (!first)
					os << ',';
				os << n;
				first=false;
			}
		}
		break;
	}
	return os;
}

/**
Based on capacity and the number of documents we're storing, choose the type.
*/
DocSet::set_type DocSet::mostEfficientType() const
{
	return bitmap; // TODO: don't always return bitmap, do the math
}

void DocSet::convertToType(set_type t)
{
	if (m_type!=t)
	{
		switch (t)
		{
		case bitmap:
		{
			// Convert from vector to bitmap
			std::vector<docid_t>::const_iterator itr_end=m_docs_arr->end();
			for (std::vector<docid_t>::const_iterator itr=m_docs_arr->begin(); itr!=itr_end; ++itr)
			{
				docid_t docid=*itr;
				m_docs_bitmap->set(docid-1);
			}
			
			m_docs_arr->clear();
			break;
		}
		case arr:
		{
			// Convert from bitmap to vector
			for (bitset_type::size_type n=m_docs_bitmap->find_first(); n!=bitset_type::npos; n=m_docs_bitmap->find_next(n))
			{
				m_docs_arr->push_back(n+1);
			}
			
			m_docs_bitmap->resize(0);
			break;
		}
		default:
			throw Exception(__FILE__,__LINE__);
			break;
		}
		
		m_type=t;
	}
}

}
