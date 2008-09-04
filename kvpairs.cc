#include <iostream>
#include <fstream>
#include <sstream>
#include <xqilla/xqilla-simple.hpp>
#include <boost/filesystem.hpp>
#include "XMLDoc.hpp"

using namespace boost::filesystem;
using namespace std;

int main(int argc, char *argv[]) {
		std::string dirname(argv[1]);
		std::string querystr(argv[2]);

		cout << "<entries>" << endl;
		
		recursive_directory_iterator end_itr;
		for (recursive_directory_iterator itr(dirname); itr!=end_itr; ++itr)
		{
			if (is_directory(itr->path()))
				continue;
				
			path xmlfile=itr->path();
			XMLDoc doc(xmlfile);
			XQueryResult result=doc.xquery(querystr);

	        while (Item::Ptr item = result.next())
			{
				cout << "<entry><key type=\"text\">" << UTF8(item->asString(result.getContext())) << "</key><file>" << xmlfile << "</file></entry>" << endl;
	        }
		}

		cout << "</entries>" << endl;
		
        return 0;
}
