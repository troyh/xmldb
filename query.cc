#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/progress.hpp>

using namespace std;
namespace bfs=boost::filesystem;

typedef	map< string, vector<uint32_t> > Index;

int main(int argc,char* argv[])
{
	boost::progress_timer timer;

	bfs::path idxfile(argv[1]);

	vector<string> terms;
	for (int i=2;i<argc;++i)
	{
		terms.push_back(argv[i]);
	}
	
	// // Find all .index files
	// vector<Index> indexes;
	// 
	// bfs::recursive_directory_iterator end_itr;
	// for (bfs::recursive_directory_iterator itr("."); itr!=end_itr; ++itr)
	// {
	// 	// cout << itr->path() << ':' << bfs::extension(itr->path()) << endl;
	// 	if (bfs::is_regular(itr->path()) && bfs::extension(itr->path())==".index")
	// 	{
	// 		cout << "Loading index:" << itr->path() << endl;
	// 		std::ifstream ifs(itr->path().string().c_str());
	// 		boost::archive::text_iarchive ar(ifs);
	// 		Index idx;
	// 		boost::serialization::load(ar,idx,0);
	// 		// boost::serialization::load(ar,indexes[indexes.size()],0);
	// 		indexes.push[indexes.size()]=idx;
	// 	}
	// }

	cout << "Loading index:" << idxfile << endl;
	std::ifstream ifs(idxfile.string().c_str());
	boost::archive::text_iarchive ar(ifs);
	Index idx;
	boost::serialization::load(ar,idx,0);
	
	boost::dynamic_bitset<> bitmap(30000);
	
	for(size_t i = 0; i < terms.size(); ++i)
	{
		// cout << terms[i] << ':';
		size_t jj=idx[terms[i]].size();
		if (jj)
		{
			for(size_t j = 0; j < jj; ++j)
			{
				// cout << (idx[terms[i]])[j] << ' ';
				bitmap[(idx[terms[i]])[j]-1]=1;
			}
		}
		else
		{
			// cout << "nil";
		}
		// cout << endl;
	}
	
	// Output bitmap bit numbers
	size_t hits=0;
	for(boost::dynamic_bitset<>::size_type p=bitmap.find_first(); p!=boost::dynamic_bitset<>::npos; p=bitmap.find_next(p))
	{
		if (hits<20)
			cout << (p+1) << ' ';
		++hits;
	}
	cout << endl;
	
	cout << hits << " hit" << (hits>1?"s":" ") << endl;
	
	return 0;
}