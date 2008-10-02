#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <glob.h>
#include "OuzoDB.hpp"

// TODO: support reindex command
// TODO: accept files relative to Ouzo config's documents/dir

using namespace std;
using namespace boost::algorithm;
namespace bfs=boost::filesystem;

void usage()
{
	cout << "ouzo <command> [arguments]" << endl
		 << endl
		<< "where <command> is one of the following:" << endl
		<< "add\tAdd document(s) to Ouzo" << endl
		<< "del\tDelete document from Ouzo" << endl
		<< "info\tOuzo info report" << endl
		<< endl;
}

int main(int argc,char* argv[])
{
	if (argc<2)
	{
		usage();
		return -1;
	}
	
	try
	{
		string cmd(argv[1]);
		to_lower(cmd);
	
		if (cmd=="add")
		{
			if (argc<3)
			{
				usage();
				return -1;
			}
			
			try
			{
				Ouzo::Ouzo ouzo("ouzo.conf");

				std::cout << "Before:" << std::endl << ouzo << std::endl;
				
				for(size_t arg = 2; arg < argc; ++arg)
				{
					// Do file-globbing
					glob_t globbuf;
					if (!glob(argv[arg],GLOB_TILDE_CHECK,NULL,&globbuf))
					{
						for (size_t i=0;i<globbuf.gl_pathc;++i)
						{
							std::cout << globbuf.gl_pathv[i] << endl;
							ouzo.addDocument(globbuf.gl_pathv[i]);
						}
						globfree(&globbuf);
					}
				}
				std::cout << "After:" << std::endl << ouzo << std::endl;
			}
			catch (...)
			{
				throw;
			}
		}
		else if (cmd=="del")
		{
			try
			{
				Ouzo::Ouzo ouzo("ouzo.conf");

				std::cout << "Before:" << std::endl << ouzo << std::endl;
				ouzo.delDocument(argv[2]);
				std::cout << "After:" << std::endl << ouzo << std::endl;
			}
			catch (...)
			{
				throw;
			}
		}
		else if (cmd=="info")
		{
			try
			{
				Ouzo::Ouzo ouzo("ouzo.conf");

				std::cout << ouzo << std::endl;
			}
			catch (...)
			{
				throw;
			}
		}
		else
		{
			usage();
			return -1;
		}
	
	}
	catch (Ouzo::Exception& x)
	{
		std::cerr << "Ouzo::Exception:" << std::endl;
		std::cerr << x.file() << '(' << x.line() << ')' << std::endl;
	}
	catch (Ouzo::Exception* x)
	{
		std::cerr << "Ouzo::Exception:" << std::endl;
		std::cerr << x->file() << '(' << x->line() << ')' << std::endl;
	}
	catch (std::exception& x)
	{
		std::cerr << "std::exception" << std::endl;
	}
	// catch (...)
	// {
	//        void *array[10];
	//        size_t size;
	//        char **strings;
	//        size_t i;
	// 
	//        size = backtrace (array, 10);
	//        strings = backtrace_symbols (array, size);
	// 
	//        printf ("Obtained %zd stack frames.\n", size);
	// 
	//        for (i = 0; i < size; i++)
	//           printf ("%s\n", strings[i]);
	// 
	//        free (strings);
	// }
	
	return 0;
}

