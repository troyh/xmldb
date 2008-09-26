#include "OuzoDB.hpp"

int main(int argc,char* argv[])
{
	try
	{
		Ouzo::Ouzo ouzo("ouzo.conf");

		ouzo.addDocument(argv[1]);
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

