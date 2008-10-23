#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>

using namespace std;

string& htmlize(string& s)
{
	struct listtype
	{ 
		const char* s;
		const char* r;
	} list[]=
	{
		{"&", "&amp;"},
		{"<", "&lt;" },
		{">", "&gt;" }
	};

	for(size_t i = 0; i < (sizeof(list)/sizeof(list[0])); ++i)
	{
		string::size_type n=0;
		while (n!=string::npos)
		{
			n=s.find(list[i].s,n);
			if (n!=string::npos)
			{
				s.replace(n,strlen(list[i].s),list[i].r);
				++n;
			}
		}
	}
	
	return s;
}

int main(int argc, char* argv[])
{
	string ln;
	
	vector<string> tags;

	// Read the 1st line to get the column names, i.e., the tag names
	getline(cin,ln);
	stringstream ss(ln);
	while (ss.good())
	{
		string tag;
		ss >> tag;
		tags.push_back(tag);
	}

	while (getline(cin,ln))
	{
		// cout << ln << endl;
		ss.clear();
		ss.str(ln);

		string val;
		vector<string> vals;
		while (getline(ss,val,'\t'))
		{
			htmlize(val);
			vals.push_back(val);
		}
		
		if (vals.size()!=tags.size())
		{
			// cout << "Bad line:(" << vals.size() << ',' << tags.size() << ")" << ln << endl;
			for(size_t i = vals.size(); i < tags.size(); ++i)
			{
				vals.push_back("");
			}
		}
		
		cout << "<" << argv[1] << ">";
		for(size_t i = 0; i < tags.size(); ++i)
		{
			cout << '<' << tags[i] << '>' << vals[i] << "</" << tags[i] << '>';
		}
		cout << "</" << argv[1] << ">" << endl;
	}
	
	return 0;
}