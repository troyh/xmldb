#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include <unistd.h>
#include <fstream>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace std;
using namespace boost::interprocess;

void write(char c)
{
	for(size_t i = 0; i < 1000; ++i)
	{
		file_lock f_lock("lockfile");
		scoped_lock<file_lock> e_lock(f_lock);

		ofstream f("lockfile",ios_base::app|ios_base::out);
		if (!f.good())
			cout << time(0) << " Writer: Unable to open lockfile for writing" << endl;
		else
		{
			for(size_t i = 0; i < 100; ++i)
			{
				f << c;
				
				// int n=rand()%10;
				// cout << "Reader: Sleeping for " << ((double)n/1000000) << 's' << endl;
				// usleep(n);
			}
			f << endl;
			f.flush();
		}
		int n=rand()%100;
		// cout << "Reader: Sleeping for " << ((double)n/1000000) << 's' << endl;
		usleep(n);

	}
}

int main(int argc, char* argv[])
{
	srand(time(0));
	
	{ // reset file to zero bytes
		ofstream f("lockfile",ios_base::trunc|ios_base::out);
	}
	
	pid_t pid1=fork();
	if (pid1) // Parent
	{
		pid_t pid2=fork();
		if (pid2)
		{
			// sleep(10);
			// kill(pid1,SIGKILL);
			// kill(pid2,SIGKILL);
			int status;
			waitpid(pid1,&status,0);
			waitpid(pid2,&status,0);
			cout << "Finished." << endl;
		}
		else
		{
			write('1');
		}
	}
	else // Child
	{
		write('0');
	}

	return 0;
}