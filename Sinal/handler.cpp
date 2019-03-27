#include<iostream>
#include<cstdlib>
#include<unistd.h>
#include<signal.h>
using namespace std;

#define KILL_SIG 1
#define STOP_SIG 2
#define CONT_SIG 3


void tp1SignalHandler(int sigNumber){
    // Function for costumized signal processing
    cout << "Received signal " << sigNumber << ". ";
    switch(sigNumber){
        case KILL_SIG:
            cout << "Killing process." << endl;
            exit(0);
            break;
        case STOP_SIG:
            cout << "Stop execution." << endl;
            break;
        case CONT_SIG:
            cout << "Continue execution." << endl;
            break;
        default:
            cout << "No meaning. Ignoring sinal." << endl;
            break;
    }
}


int main(int argc, char** argv){
    int waitType;

    if (argc < 2){
        cout << "WARNING: No waiting type defined. Default for 'busy'." << endl;
        cout << "         Possible arguments: 'busy' (busy wait) or 'block' "
             << "(blocking wait)." << endl;
        waitType = 0;
    } else {
        if ( strcmp(argv[1], "busy") == 0 )
            waitType = 0;
        else if ( strcmp(argv[1], "block") == 0 )
            waitType = 1;
        else {
            cout << "WARNING: Argument not recognized. Set to 'busy'." << endl;
            waitType = 0;
        }
    }

    signal( KILL_SIG, tp1SignalHandler );
    signal( STOP_SIG, tp1SignalHandler );
    signal( CONT_SIG, tp1SignalHandler );

    while(1){
        if (waitType == 1){
            cout << "Waiting without overusing the machine" << endl;
            sleep(100);
        }
        else{
            // cout << "Busily waiting for signal" << endl;
            continue;
        }
    }

    return 0;
}