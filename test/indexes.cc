#include <getopt.h>
#include <math.h>
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>

#include "Ouzo.hpp"

using namespace std;
using namespace Ouzo;
using namespace boost;

Index::key_t::key_type possible_types[]={
	Index::key_t::KEY_TYPE_INT8,
	Index::key_t::KEY_TYPE_INT16,
	Index::key_t::KEY_TYPE_INT32,
	Index::key_t::KEY_TYPE_INT64,
	Index::key_t::KEY_TYPE_UINT8,
	Index::key_t::KEY_TYPE_UINT16,
	Index::key_t::KEY_TYPE_UINT32,
	Index::key_t::KEY_TYPE_UINT64,
	Index::key_t::KEY_TYPE_DBL,
	Index::key_t::KEY_TYPE_CHAR8,
	Index::key_t::KEY_TYPE_DATE,
	Index::key_t::KEY_TYPE_TIME,
	Index::key_t::KEY_TYPE_FLOAT,
	Index::key_t::KEY_TYPE_STRING
};

Index::key_t::key_type types[sizeof(possible_types)/sizeof(possible_types[0])];

void usage()
{
	cout << "Usage: indexes <string|uint32|uint16|...>" << endl;
}

int main(int argc, char* argv[])
{
	int bDoAllTypes=false;
	Index::key_t::key_type type=Index::key_t::KEY_TYPE_UNKNOWN;
	size_t capacity=10000;
	size_t max_keys=100;
	size_t types_to_do=0;
	
	static struct option long_options[]=
	{
		{ "all"     , optional_argument, &bDoAllTypes, true },
		{ "type"    , required_argument, NULL, 't' },
		{ "capacity", required_argument, NULL, 'c' },
		{ "maxkeys" , required_argument, NULL, 'k' },
		{ 0, 0, 0, 0 }
	};
	
	int option_index=0;
	int c;
	while ((c=getopt_long_only(argc,argv,"at:ck",long_options,&option_index))!=-1)
	{
		switch(c)
		{
		case 0:
			break;
		case 'a':
			break;
		case 't':
			type=Index::key_t::getKeyType(optarg);
			if (type==Index::key_t::KEY_TYPE_UNKNOWN)
			{
				cerr << "Unknown index type: " << optarg << endl;
			}
			break;
		case 'c':
			capacity=strtoul(optarg,0,10);
			break;
		case 'k':
			max_keys=strtoul(optarg,0,10);
			break;
		case '?':
			if (optopt=='t')
			{
				// cerr << "-t type not specified" << endl;
			}
			break;
		default:
			abort();
			break;
		}
	}
	
	
	if (optind<argc)
	{
		for (;optind<argc;++optind)
			cerr << "Ignored argument: " << argv[optind] << endl;
		return -4;
	}

	if (bDoAllTypes)
	{
		if (type!=Index::key_t::KEY_TYPE_UNKNOWN)
		{
			cerr << "Mutually-exclusive options: --all and --type. Please specify one or the other." << endl;
			return -1;
		}
		
		for(size_t i = 0; i < (sizeof(possible_types)/sizeof(possible_types[0])); ++i)
		{
			types[i]=possible_types[i];
		}
		types_to_do=sizeof(possible_types)/sizeof(possible_types[0]);
	}
	else if (type==Index::key_t::KEY_TYPE_UNKNOWN)
	{
		cerr << "Either --all or --type must be specified." << endl;
		return -2;
	}
	else
	{
		types_to_do=1;
		types[0]=type;
	}
	
	
	// Create each type of index
	srand(time(0));
	srand48(time(0));
	
	for (size_t i=0; i < types_to_do; ++i)
	{
		cout << "new " << types[i] << "_test " << types[i] << ' ' << capacity << ';' << endl;
		
		// Randomly put keys and docids
		
		size_t nkeys_count=(rand()%max_keys)+1;
		for(size_t nkeys = 0; nkeys < nkeys_count; ++nkeys)
		{
			ostringstream ss;

			if (types[i]==Index::key_t::KEY_TYPE_STRING || types[i]==Index::key_t::KEY_TYPE_CHAR8)
			{
				// Create a random key of letters and digits
		
				size_t nj=(rand()%31)+1;
				if (types[i]==Index::key_t::KEY_TYPE_CHAR8)
					nj=(rand()%8)+1;
					
				for(size_t j = 0; j < nj; ++j)
				{
					int n=rand()%36;
					if (n>25)
						ss << (char)('0'+(n-26));
					else
						ss << (char)('a'+n);
				}
			}
			else if (types[i]==Index::key_t::KEY_TYPE_INT8)  { ss << (int     )(rand()%255-(128)); }
			else if (types[i]==Index::key_t::KEY_TYPE_INT16) { ss << (int16_t )mrand48(); }
			else if (types[i]==Index::key_t::KEY_TYPE_INT32) { ss << (int32_t )mrand48(); }
			else if (types[i]==Index::key_t::KEY_TYPE_INT64) { ss << (int64_t )((((int64_t)mrand48())<<32)|rand()); }
			else if (types[i]==Index::key_t::KEY_TYPE_UINT8) { ss << (uint16_t)(rand()%255); }
			else if (types[i]==Index::key_t::KEY_TYPE_UINT16){ ss << (uint16_t)lrand48(); }
			else if (types[i]==Index::key_t::KEY_TYPE_UINT32){ ss << (uint32_t)(lrand48()<<1); }
			else if (types[i]==Index::key_t::KEY_TYPE_UINT64){ ss << (uint64_t)((((uint64_t)lrand48())<<32)|rand()); }
			else if (types[i]==Index::key_t::KEY_TYPE_DBL)   { ss << (double  )(drand48()*mrand48()); }
			else if (types[i]==Index::key_t::KEY_TYPE_FLOAT) { ss << (float   )(drand48()*mrand48()); }
			else if (types[i]==Index::key_t::KEY_TYPE_DATE)  { ss << (((rand()%200+1900)*10000)+((rand()%12+1)*100)+(rand()%31+1)); }
			else if (types[i]==Index::key_t::KEY_TYPE_TIME)  { ss << lrand48(); }
			
			string key=ss.str();
		
			cout << "put " << types[i] << "_test";
		
			size_t nputs=10;
			for(size_t put = 0; put < nputs; ++put)
			{
				cout << endl << '(' << key << ':';
			
				size_t ndocs=lrand48()%((uint32_t)((double)capacity*0.01)); // up to 1% of total possible docids will be set on each key
				for (size_t d=0; d<ndocs; ++d)
				{
					docid_t docid=(lrand48()%capacity)+1;
					cout << docid << ' ';
				}
		
				cout << ')';
			}
			cout << ';' << endl;
		}
		
	}
	
	return 0;
}