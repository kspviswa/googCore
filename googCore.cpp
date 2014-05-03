/*
 * googCore.cpp
 *
 *  Created on: May 3, 2014
 *      Author: viswa
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <google/coredumper.h>
#include <signal.h>  // for handling signal
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sstream>
#include <limits>
#include <errno.h>


using namespace std;

#define NT_VISWA 0x46494c46 // NT_FILE + 1

string toString(int num)
{
	std::stringstream out;
	out << num;
	return(out.str());
}

void signal_handler(int SIGNO)
{
    cout << "Caught Signal" << endl;
    cout << "Attempting to dump core" << endl;

    // Get the mmap of the process
    char data[5000] = {0};
    char *pStart, *pInit, *pEnd;
    pInit = pStart = data;
    pEnd = pInit + 5000;
    int pid = getpid();

    int status;

    char *pMapped;
    struct stat fileStat;
    int fd = 0;
    string sPidMMAPFile = "/proc/"+toString(pid)+"/maps";
    long fsize;

    string line;
    ifstream pMapStream (sPidMMAPFile.c_str());

    if (pMapStream.is_open())
    {
    	string nL("\n");
    	// DEBUG - cout << "Opened file" << sPidMMAPFile << endl;
    	 while ( getline (pMapStream,line) )
    	 {
    		// cout << "Reading line " << line << endl;
    		 memcpy(pStart, line.c_str(), strlen(line.c_str()));
    		 pStart += strlen(line.c_str());
    		 memcpy(pStart, nL.c_str(), strlen(nL.c_str()));
    		 pStart += strlen(nL.c_str());

    	 }
    	 pMapStream.close();
    }
    else
    {
    	cout << "Error in opening" << sPidMMAPFile << endl;
    	exit(-1);
    }

    cout << "data has been copied" << endl;

    fsize = pStart - pInit;

    // now lets proceed to dump the core
    struct CoreDumpParameters theParam;
    struct CoredumperNote theNote;

    theNote.name = "kspviswa-special-notes-mmap";
    theNote.type = NT_VISWA;
    theNote.description_size = fsize;
    theNote.description = pInit;

    // DEBUG - cout << "Proceeding with GC with notes size" << theNote.description_size << endl;
    // GC
    ClearCoreDumpParameters(&theParam);
    SetCoreDumpNotes(&theParam, &theNote, 1);
    status = WriteCoreDumpWith(&theParam, "viswa-core");

    if(status != 0)
    {
    	char error[128];
    	strerror_r( errno, error, 128 );
    	cout << "Unable to dump-core, due to " << strerror(errno) << endl;
    }
    else
    {
    	cout << "Successfully dumped core ==> viswa-core" << endl;
    }
    exit(0);
}

int main()
{
	signal(SIGSEGV, signal_handler);
	cout << "Starting the program with pid" << getpid() << endl;
	int c;
	cout << "OK am now waiting for the signal" << endl;
	while(1);
	return 0;
}


