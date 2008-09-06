#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;
namespace bfs=boost::filesystem;

typedef	map< string, vector<uint32_t> > Index;

int main(int argc,char* argv[])
{
	vector<string> terms;
	for (int i=1;i<argc;++i)
	{
		terms.push_back(argv[i]);
	}
	
	// Find all .index files
	vector<Index> indexes;
	
	bfs::recursive_directory_iterator end_itr;
	for (bfs::recursive_directory_iterator itr("."); itr!=end_itr; ++itr)
	{
		// cout << itr->path() << ':' << bfs::extension(itr->path()) << endl;
		if (bfs::is_regular(itr->path()) && bfs::extension(itr->path())==".index")
		{
			cout << "Loading index:" << itr->path() << endl;
			std::ifstream ifs(itr->path().string().c_str());
			boost::archive::text_iarchive ar(ifs);
			boost::serialization::load(ar,indexes[indexes.size()],0);
		}
	}
	
	
	for(size_t i = 0; i < terms.size(); ++i)
	{
		cout << terms[i] << endl;
	}
	
	return 0;
}