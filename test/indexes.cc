#include <string>
#include <boost/filesystem.hpp>

#include "StringIndex.hpp"

using namespace std;
using namespace Ouzo;
using namespace boost;

const size_t NUM_DOCS=1000;

void usage()
{
	cout << "Usage: indexes <string|uint32|uint16|...>" << endl;
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
		cout << itr->first << ':' << itr->second.count() << endl;
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
		cout << (ptm->tm_year+1900) << '-' << (ptm->tm_mon+1) << '-' << ptm->tm_mday << ':' << itr->second.count() << endl;
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
		cout << (ptm->tm_year+1900) << '-' << (ptm->tm_mon+1) << '-' << ptm->tm_mday << ' ' << ptm->tm_hour << ':' << ptm->tm_min << ':' << ptm->tm_sec << " :" << itr->second.count() << endl;
	}
}

void showkeys(StringIndex* idx)
{
	cout << "Keys:" << endl;
	StringIndex::const_iterator_type itr_end=idx->end();
	for (StringIndex::const_iterator_type itr=idx->begin(); itr!=itr_end; ++itr)
	{
		const DocSet& ds=idx->get(itr->first.c_str());
		cout << itr->first << ':' << ds.count() << endl;
	}
}


void readFromFile(Index* idx, filesystem::path fname)
{
	ifstream f(fname.string().c_str());
	while (f.good())
	{
		string key;
		docid_t docid;
		f >> key >> docid;
		idx->put(key,docid);
	}
}


int main(int argc, char* argv[])
{
	if (argc<2)
	{
		usage();
		return -1;
	}
	
	// srand(time(0));
	
	string cmd=argv[1];
	
	if (cmd=="list")
	{
		Index* idx=Index::loadIndexFromFile(argv[2]);
		cout << *idx << endl;
	}
	else if (cmd=="create")
	{
		string type=argv[2];
		string index_name=argv[3];
		
		Index* idx;

		if (type=="string")
		{
			idx=new StringIndex(index_name, index_name, "foobar", NUM_DOCS);
			readFromFile(idx,argv[3]);
			idx->save();

			// idx->put("zzxqqqoksu",42);
	
			// for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
			// {
			// 	string randomkey;
			// 	for(size_t i = 0; i < 10; ++i)
			// 	{
			// 		randomkey.push_back('a'+(rand()%26));
			// 	}
			// 	idx->put(randomkey.c_str(),docid);
			// }
			
		}
		else if (type=="date")
		{
			idx=new DateIndex(index_name, index_name, "foobar", NUM_DOCS);
		
			// idx->load();
			// 		
			// for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
			// {
			// 	time_t randomkey=time(0)+(rand()-(.5*rand()));
			// 	idx->put(randomkey,docid);
			// }
			// 		
			// idx->save();
			// 		
			// showkeys(idx);
		}
		else if (type=="time")
		{
			idx=new TimeIndex(index_name, index_name, "foobar", NUM_DOCS);
			// idx->load();
			// 		
			// for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
			// {
			// 	time_t randomkey=time(0)+(rand()-(.5*rand()));
			// 	idx->put(randomkey,docid);
			// }
			// 		
			// idx->save();
			// 
			// showkeys(idx);
		}
		else if (type=="float")
		{
			idx=new FloatIndex(index_name, index_name, "foobar", NUM_DOCS);
			// idx->load();
			// 		
			// for(docid_t docid = 1; docid <= NUM_DOCS; ++docid)
			// {
			// 	double randomkey=((double)rand())/rand();
			// 	idx->put(randomkey,docid);
			// }
			// 		
			// idx->save();
			// 		
			// showkeys(idx);
		}
		else if (type=="uint32")
		{
			idx=new UIntIndex<uint32_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		else if (type=="uint16")
		{
			idx=new UIntIndex<uint16_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		else if (type=="uint8")
		{
			idx=new UIntIndex<uint8_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		else if (type=="int32")
		{
			idx=new UIntIndex<int32_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		else if (type=="int16")
		{
			idx=new UIntIndex<int16_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		else if (type=="int8")
		{
			idx=new UIntIndex<int8_t>(index_name, index_name, "foobar", NUM_DOCS);
			// addkeys(idx);
			// showkeys(idx);
		}
		
		readFromFile(idx,argv[3]);
		idx->save();
		
	}
	
	return 0;
}