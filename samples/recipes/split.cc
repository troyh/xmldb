#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[])
{
	string ln;
	size_t n=1;
	
	while (getline(cin,ln))
	{
		ostringstream ss;
		ss << argv[1] << '/' << n << ".xml";
		
		cout << "File:" << ss.str() << endl;
		ofstream f(ss.str().c_str());
		f << ln;
		
		++n;
	}
	
	return 0;
}