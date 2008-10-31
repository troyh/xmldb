#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <glob.h>
#include "Ouzo.hpp"

// TODO: support reindex command

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

				// std::cout << "Before:" << std::endl << ouzo << std::endl;

				// This is kinda dumb to do the glob-ing twice, but I want to get a count of
				// the number of files for all the args to display a progress meter.
								
				size_t count=0;
				for(size_t arg = 2; arg < argc; ++arg)
				{
					// Do file-globbing
					glob_t globbuf;
					if (!glob(argv[arg],GLOB_TILDE_CHECK,NULL,&globbuf))
					{
						count+=globbuf.gl_pathc;
					}
				}
				
				std::cout << "Adding " << count << " documents" << std::endl;
				boost::progress_display progress(count	);
				
				for(size_t arg = 2; arg < argc; ++arg)
				{
					// Do file-globbing
					glob_t globbuf;
					if (!glob(argv[arg],GLOB_TILDE_CHECK,NULL,&globbuf))
					{
						for (size_t i=0;i<globbuf.gl_pathc;++i)
						{
							// Make the filepath absolute
							bfs::path fpath(globbuf.gl_pathv[i]);
							if (!fpath.has_root_path())
							{
								// Use the cwd
								fpath=bfs::initial_path() / fpath;
							}

							// std::cout << fpath << endl;
							ouzo.addDocument(fpath);
							++progress;
						}
						globfree(&globbuf);
					}
				}
				// std::cout << "After:" << std::endl << ouzo << std::endl;
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
		else if (cmd=="query")
		{
			try
			{
				Ouzo::Ouzo ouzo("ouzo.conf");
				
				Ouzo::DocumentBase* pDB=ouzo.getDocBase("recipe_reviews");
				Ouzo::Query::Results results(pDB);
				
				std::string dbname("recipe_reviews");
				
				std::string key("chef_id");
				std::string val("2312");
				Ouzo::Query::TermNode* query=new Ouzo::Query::TermNode(dbname,key,Ouzo::Query::TermNode::eq,val);
				
				std::string key2("review_rating");
				std::string val2("5");
				Ouzo::Query::TermNode* query2=new Ouzo::Query::TermNode(dbname,key2,Ouzo::Query::TermNode::eq,val2);
				
				Ouzo::Query::BooleanNode* boolop=new Ouzo::Query::BooleanNode(dbname,Ouzo::Query::BooleanNode::AND);
				boolop->left(query);
				boolop->right(query2);
				
				ouzo.fetch(*boolop, results);
				
				std::vector<bfs::path> docs;
				pDB->getDocFilenames(results, docs);

				std::istream::fmtflags old_flags = std::cout.setf( std::istream::fixed,std::istream::floatfield );
				std::streamsize old_prec = std::cout.precision( 4 );

				std::cout << "<results count=\"" << docs.size() << "\" querytime=\"" << results.queryTime() << "\">" << std::endl;
				
				std::cout.flags( old_flags );
				std::cout.precision( old_prec );

				for(size_t i = 0; i < docs.size(); ++i)
				{
					bfs::path fname=pDB->docDirectory() / docs[i];
					ifstream f(fname.string().c_str());
					string buf;
					while (getline(f,buf))
						std::cout << buf << std::endl;
				}
				std::cout << "</results>" << std::endl;

				delete boolop;
				
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

				if (argc>2)
				{
					string subcmd(argv[2]);
					if (subcmd=="index")
					{
						if (argc>3)
						{
							string docbase(argv[3]);
							Ouzo::DocumentBase* pDB=ouzo.getDocBase(docbase);
							if (pDB)
							{
								if (argc>4)
								{
									string idxname(argv[4]);
									Ouzo::Index* pIdx=pDB->getIndex(idxname);
									if (pIdx)
									{
										pIdx->load();
										std::cout << *pIdx << std::endl;
									}
								}
								else
								{
									std::cout << "DocumentBase:" << std::endl;
									std::cout << *pDB << std::endl;
								}
							}
						}
					}
				}
				else
				{
					std::cout << ouzo << std::endl;
				}
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
		// std::cerr << "Ouzo::Exception:" << std::endl;
		// std::cerr << x.file() << '(' << x.line() << ')' << std::endl << std::endl;
		std::cerr << x << std::endl;
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

