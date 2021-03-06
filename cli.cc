#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <glob.h>
#include <getopt.h>

#include <sstream>

#include "Ouzo.hpp"
#include "Index.hpp"

extern "C"
{
#include "cli.h"
}

// TODO: support reindex command

extern char* linebuf;
extern unsigned int lineno;
extern unsigned int tokenpos;


using namespace std;
using namespace boost::algorithm;
namespace bfs=boost::filesystem;

istream* lex_input=NULL;
bool bQuit=false;

extern "C"
int my_yyinput(char* buf, int max_size)
{
	if (lex_input->bad())
		return 0;
		
	int n;
	
	if (bQuit)
	{
		n=0;
	}
	else
	{
		if (lex_input==&cin && isatty(0))
			cout << "ouzo>";
		
		lex_input->getline(buf,max_size);
		n=lex_input->gcount();
	}
	
	if (!n)
	{
		if (lex_input==&cin && isatty(0))
		{
			cout << endl << "Arf!" << endl;
		}
		bQuit=true;
		return 0;
	}
	
	return n-1;
}

extern "C" 
void yyerror(const char *str)
{
	cerr << "Error on line " << lineno << " column " << tokenpos << ": " << str << endl
	 	 << linebuf << endl;
	fprintf(stderr,"%*s\n",1+tokenpos,"^");
}

extern "C" 
int yywrap()
{
	if (bQuit)
		return 1;
	return lex_input->good()?0:1;
} 



void usage()
{
	cout << "ouzo [options]" << endl
		 << endl
		<< "where [options] is one or more of the following:" << endl
		<< "-e,--expression	      Specify an Ouzo expression to be run non-interactively." << endl
		<< "-h,--help             Print this." << endl
		<< endl;
}

extern "C" int yyparse();

extern "C"
void ouzo_new_index(const char* name, const char* type, size_t capacity)
{
	Ouzo::Index* idx=Ouzo::Ouzo::createIndex(Ouzo::Index::key_t::getKeyType(type),name,"",capacity);

	bfs::path fpath="./"; // Current dir is data dir
	fpath /= name;
	idx->setFilename(fpath);
	idx->initFile();
	// cout << *idx << endl;
	idx->save();
	delete idx;
}

extern "C"
void ouzo_show_index(const char* name)
{
	try
	{
		Ouzo::Index* idx=Ouzo::Index::loadFromFile(name);
		cout << *idx << endl;
		delete idx;
	}
	catch (Ouzo::Exception& x)
	{
		cerr << x << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception." << endl;
	}
}

extern "C"
void ouzo_index_get(const char* name, STRING_LIST* list)
{ 
	Ouzo::Index* idx=Ouzo::Index::loadFromFile(name);

	while (list)
	{
		size_t i;
		for (i=0;i<list->count;++i)
		{
			Ouzo::Index::key_t* key=idx->createKey();
			key->assign(list->list[i]);
			const Ouzo::DocSet& ds=idx->get(*key);
			cout << *key << ':' << ds << endl;
			
			free(list->list[i]);
			
			delete key;
		}

		STRING_LIST* freelist=list;
		list=list->next;
		
		free(freelist);
	}
	
	idx->save();
	
	delete idx;
}

extern "C"
void ouzo_index_put(const char* name, KEY_DOCID_LIST* list)
{ 
	Ouzo::Index* idx=Ouzo::Index::loadFromFile(name);
	
	while (list)
	{
		Ouzo::Index::key_t* k=idx->createKey();
		k->assign(list->key);
		// cout << "ouzo_index_put:" << list->key << endl;

		NUMBER_LIST* p=list->docids;
		for (; p; p=p->next)
		{
			size_t i;
			for (i=0;i<p->count;++i)
			{
				Ouzo::docid_t docid=p->list[i];
				idx->put(*k,docid);
			}
		}
		
		delete k;
		
		// Free them as we go, and move to the next one
		KEY_DOCID_LIST* freelist=list;
		list=list->next;
		free(freelist);
	}
	
	idx->save();

	delete idx;
}

extern "C"
void ouzo_index_unput(const char* name, KEY_DOCID_LIST* list)
{ 
	Ouzo::Index* idx=Ouzo::Index::loadFromFile(name);

	while (list)
	{
		printf("UNPUT:%s:%s:",name,list->key);
		NUMBER_LIST* p=list->docids;
		for (; p; p=p->next)
		{
			size_t i;
			for (i=0;i<p->count;++i)
			{
				printf("%c%d",(i?',':' '),p->list[i]);
			}
		}
		printf("\n");
	
		KEY_DOCID_LIST* freelist=list;
	
		list=list->next;
	
		free(freelist);
	}
	
	idx->save();
	
	delete idx;
}

extern "C"
void cli_quit()
{
	bQuit=true;
}

void ouzo_docbase_put_progress(void* p)
{
	boost::progress_display* pd=(boost::progress_display*)p;
	++(*pd);
}

extern "C"
void ouzo_docbase_put(const char* filename)
{ 
	try
	{
		Ouzo::Ouzo ouzo("ouzo.conf");
		// std::cout << "Before:" << std::endl << ouzo << std::endl;

		std::vector<bfs::path> files;
		
		// Do file-globbing
		glob_t globbuf;
		if (!glob(filename,GLOB_TILDE_CHECK,NULL,&globbuf))
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
				
				files.push_back(fpath);
			}
			globfree(&globbuf);
		}
		
		std::cout << "Adding " << files.size() << " documents" << std::endl;
		
		boost::progress_display progress(files.size());
		ouzo.addDocument(files,&ouzo_docbase_put_progress,&progress);
	}
	catch (Ouzo::Exception& x)
	{
		std::cerr << x << std::endl;
	}
	catch (...)
	{
		throw;
	}
}

extern "C"
void ouzo_docbase_del(const char* filename)
{
	try
	{
		Ouzo::Ouzo ouzo("ouzo.conf");
		
		// Add CWD to filename if it's not there
		bfs::path f(filename);
		if (!f.has_root_path())
		{
			// Use the cwd
			f=bfs::initial_path() / filename;
		}
		
		ouzo.delDocument(f);
	}
	catch (Ouzo::Exception& x)
	{
		std::cerr << x << std::endl;
	}
	catch (...)
	{
		throw;
	}
	
}

extern "C"
void ouzo_info()
{
	// try
	// {
	// 	Ouzo::Ouzo ouzo("ouzo.conf");
	// 	if (argc>2)
	// 	{
	// 		string subcmd(argv[2]);
	// 		if (subcmd=="index")
	// 		{
	// 			if (argc>3)
	// 			{
	// 				string docbase(argv[3]);
	// 				Ouzo::DocumentBase* pDB=ouzo.getDocBase(docbase);
	// 				if (pDB)
	// 				{
	// 					if (argc>4)
	// 					{
	// 						string idxname(argv[4]);
	// 						Ouzo::Index* pIdx=pDB->getIndex(idxname);
	// 						if (pIdx)
	// 						{
	// 							pIdx->load();
	// 							std::cout << *pIdx << std::endl;
	// 						}
	// 					}
	// 					else
	// 					{
	// 						std::cout << "DocumentBase:" << std::endl;
	// 						std::cout << *pDB << std::endl;
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		std::cout << ouzo << std::endl;
	// 	}
	// }
	// catch (Ouzo::Exception& x)
	// {
	// 	cerr << x << endl;
	// }
	// catch (...)
	// {
	// 	throw;
	// }
	
}

extern "C"
void ouzo_query()
{
	try
	{
		boost::timer total_timer;
		
		Ouzo::Ouzo ouzo("ouzo.conf");
		double init_time=total_timer.elapsed();
		
		Ouzo::DocumentBase* pDB=ouzo.getDocBase("recipes");
		Ouzo::Query::Results results(pDB);
		
		std::string dbname("recipes");
		
		// std::string idxname("chef_id");
		// Ouzo::Index* idx=pDB->getIndex(idxname);
		// Ouzo::uint32key_t val(2312);

		std::string idxname("recipe_id");
		Ouzo::Index* idx=pDB->getIndex(idxname);
		// Ouzo::Index::key_t* key=idx->createKey();
		// key->assign("10022");
		// key->assign("102570");
		
		// query index directly
		// const Ouzo::DocSet& ds=idx->get(*key);
		// cout << "Direct index query: "<< *key << ':' << ds << endl;
		
		Ouzo::StringIndex::stringkey_t val;
		val.assign("102570");
		// val.assign("10010");
		// val.assign("10022");
		// val.assign("10043");

		// TODO: figure out why having cerr.flush() in the binary at all causes the
		// correct results for string queries, even if it's never executed as in this
		// case. But comment that 1 line out and the above stringkey_t query gives an
		// incorrect result.
				
		if ((int)idx==1) // Never happen
		{
			// cerr.flush();
		}
		

		Ouzo::Query::TermNode* query=new Ouzo::Query::TermNode(dbname,idxname,Ouzo::Query::TermNode::eq,val);
		
		// Ouzo::DocSet& ds=idx->get(val);
		// cout << "Hits:" << ds.count() << endl;
		
		// std::string idxname2("review_rating");
		// idx=pDB->getIndex(idxname2);
		// Ouzo::uint8key_t val2(5);
		// Ouzo::Query::TermNode* query2=new Ouzo::Query::TermNode(dbname,idxname2,Ouzo::Query::TermNode::eq,val2);
		// 
		// Ouzo::Query::BooleanNode* boolop=new Ouzo::Query::BooleanNode(dbname,Ouzo::Query::BooleanNode::AND);
		// boolop->left(query);
		// boolop->right(query2);
		
		boost::timer query_timer;
		ouzo.fetch(*query, results);
		double query_time=query_timer.elapsed();
		
		std::vector<bfs::path> docs;
		pDB->getDocFilenames(results, docs);

		std::istream::fmtflags old_flags = std::cout.setf( std::istream::fixed,std::istream::floatfield );
		std::streamsize old_prec = std::cout.precision( 10 );

		std::cout << "<results count=\"" << docs.size() << "\" querytime=\"" << results.queryTime() << "\">" << std::endl;
		
		boost::timer readdoc_timer;
		for(size_t i = 0; i < docs.size(); ++i)
		{
			bfs::path fname=pDB->docDirectory() / docs[i];
			ifstream f(fname.string().c_str());
			string buf;
			while (getline(f,buf))
				std::cout << buf << std::endl;
		}
		std::cout << "</results>" << std::endl;

		std::cout << "<!-- Init time  : " << init_time << std::endl
		          << "     Query time : " << query_time << std::endl
		          << "     Output time: " << readdoc_timer.elapsed() << std::endl
		          << "     Total time : " << total_timer.elapsed() << " -->" << std::endl;
		
		std::cout.precision( old_prec );
		std::cout.flags( old_flags );
	}
	catch (Ouzo::Exception& x)
	{
		std::cerr << x << std::endl;
	}
	catch (...)
	{
		throw;
	}
}


int main(int argc,char* argv[])
{
	try
	{
		lex_input=&cin;

		static struct option long_options[]=
		{
			{ "expression", required_argument, NULL, 'e' },
			{ "help"      , no_argument      , NULL, 'h' },
			{ 0, 0, 0, 0 }
		};

		int option_index=0;
		int c;
		while ((c=getopt_long_only(argc,argv,"e:h",long_options,&option_index))!=-1)
		{
			switch(c)
			{
			case 0:
				break;
			case 'e':
			{
				string s=optarg;
				s.append(";\n");
				lex_input=new istringstream(s);
				break;
			}
			case 'h':
				usage();
				return 0;
				break;
			}
		}
		
		if (optind<argc)
		{
			for (size_t i=optind; i<argc; ++i)
			{
				cerr << "Unnecessary argument:" << argv[i] << endl;
			}
			return -1;
		}
		
		while (bQuit==false)
		{
			yyparse();
		}
		
		string cmd;
		to_lower(cmd);
	
		if (cmd=="query")
		{
			try
			{
				Ouzo::Ouzo ouzo("ouzo.conf");
				Ouzo::DocumentBase* pDB=ouzo.getDocBase("recipe_reviews");
				Ouzo::Query::Results results(pDB);
				
				std::string dbname("recipe_reviews");
				
				std::string idxname("chef_id");
				Ouzo::Index* idx=pDB->getIndex(idxname);
				Ouzo::uint32key_t val(2312);
				Ouzo::Query::TermNode* query=new Ouzo::Query::TermNode(dbname,idxname,Ouzo::Query::TermNode::eq,val);
				
				std::string idxname2("review_rating");
				idx=pDB->getIndex(idxname2);
				Ouzo::uint8key_t val2(5);
				Ouzo::Query::TermNode* query2=new Ouzo::Query::TermNode(dbname,idxname2,Ouzo::Query::TermNode::eq,val2);
				
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
	
	return 0;
}

