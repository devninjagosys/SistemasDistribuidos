#include<iostream>
#include<cstdlib>
#include<signal.h>
#include <errno.h>
using namespace std;


int main(int argc, char* argv[]){
    if (argc < 3){
        cout << "ERROR: Not enough arguments." << endl;
        cout << "       This program needs to be given the target process ID "
             << "and the signal that needs to be sent." << endl;
        exit(1);
    }

    pid_t targetPID = atoi(argv[1]);
    int   sigToSend = atoi(argv[2]);

    // NOTE: Can I check *first* if the process exists?
    int success = kill(targetPID, sigToSend);

    if (success == -1){
        cout << "ERROR: Failure to send signal." << endl;
        cout << "       " << strerror(errno) << endl;
        exit(1);
    }

    cout << "Successfully sent signal " << sigToSend << " to process "
         << targetPID << endl;

    return 0;
}