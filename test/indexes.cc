#include <string>

#include "StringIndex.hpp"

using namespace std;
using namespace Ouzo;

const size_t NUM_DOCS=1000;

void usage()
{
	cout << "Usage: indexes <string|uint32>" << endl;
}

template<typename T>
void addkeys(UIntIndex<T>* idx)
{
	idx->load();

	size_t n=idx->documentCapacity();
	for(docid_t docid = 1; docid <= n; ++docid)
	{
		T randomkey=rand();
		idx->put(randomkey,docid);
	}
	
	idx->save();
}

template<typename T>
void showkeys(UIntIndex<T>* idx)
{
	cout << "Keys:" << endl;
	typename UIntIndex<T>::const_iterator_type itr_end=idx->end();
	for (typename UIntIndex<T>::const_iterator_type itr=idx->begin(); itr!=itr_end; ++itr)
	{
		cout << itr->first << endl;
	}
}

void showkeys(DateIndex* idx)
{
	cout << "Keys:" << endl;
	DateIndex::const_iterator_type itr_end=idx->end();
	for (DateIndex::const_iterator_type itr=idx->begin(); itr!=itr_end; ++itr)
	{
		time_t t=itr->first;
		struct tm* ptm=localtime(&t);
		cout << (ptm->tm_year+1900) << '-' << (ptm->tm_mon+1) << '-' << ptm->tm_mday << endl;
	}
}

void showkeys(TimeIndex* idx)
{
	cout << "Keys:" << endl;
	TimeIndex::const_iterator_type itr_end=idx->end();
	for (TimeIndex::const_iterator_type itr=idx->begin(); itr!=itr_end; ++itr)
	{
		time_t t=itr->first;
		struct tm* ptm=localtime(&t);
		cout << (ptm->tm_year+1900) << '-' << (ptm->tm_mon+1) << '-' << ptm->tm_mday << ' ' << ptm->tm_hour << ':' << ptm->tm_min << ':' << ptm->tm_sec << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc<1)
	{
		usage();
		return -1;
	}
	
	srand(time(0));
	
	string type=argv[1];
	string index_name=type;
	index_name+="_test";
	
	if (type=="string")
	{
		StringIndex* idx=new StringIndex(index_name, index_name, "foobar", NUM_DOCS);
		
		idx->load();
		
		for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
		{
			string randomkey;
			for(size_t i = 0; i < 10; ++i)
			{
				randomkey.push_back('a'+(rand()%26));
			}
			idx->put(randomkey.c_str(),docid);
		}
		
		idx->save();
		
		cout << "Keys:" << endl;
		StringIndex::const_iterator_type itr_end=idx->end();
		for (StringIndex::const_iterator_type itr=idx->begin(); itr!=itr_end; ++itr)
		{
			cout << itr->first << endl;
		}
		
	}
	else if (type=="date")
	{
		DateIndex* idx=new DateIndex(index_name, index_name, "foobar", NUM_DOCS);
		
		idx->load();
		
		for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
		{
			time_t randomkey=time(0)+(rand()-(.5*rand()));
			idx->put(randomkey,docid);
		}
		
		idx->save();
		
		showkeys(idx);
	}
	else if (type=="time")
	{
		TimeIndex* idx=new TimeIndex(index_name, index_name, "foobar", NUM_DOCS);
		idx->load();
		
		for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
		{
			time_t randomkey=time(0)+(rand()-(.5*rand()));
			idx->put(randomkey,docid);
		}
		
		idx->save();

		showkeys(idx);
	}
	else if (type=="float")
	{
		FloatIndex* idx=new FloatIndex(index_name, index_name, "foobar", NUM_DOCS);
		idx->load();
		
		for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
		{
			double randomkey=((double)rand())/rand();
			idx->put(randomkey,docid);
		}
		
		idx->save();
		
		showkeys(idx);
	}
	else if (type=="uint32")
	{
		UIntIndex<uint32_t>* idx=new UIntIndex<uint32_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	else if (type=="uint16")
	{
		UIntIndex<uint16_t>* idx=new UIntIndex<uint16_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	else if (type=="uint8")
	{
		UIntIndex<uint8_t>* idx=new UIntIndex<uint8_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	else if (type=="int32")
	{
		UIntIndex<int32_t>* idx=new UIntIndex<int32_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	else if (type=="int16")
	{
		UIntIndex<int16_t>* idx=new UIntIndex<int16_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	else if (type=="int8")
	{
		UIntIndex<int8_t>* idx=new UIntIndex<int8_t>(index_name, index_name, "foobar", NUM_DOCS);
		addkeys(idx);
		showkeys(idx);
	}
	
	return 0;
}