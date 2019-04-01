#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#define EXIT_PIPE_FAILURE EXIT_FAILURE
#define EXIT_FORK_FAILURE EXIT_FAILURE
#define TIME_SLEEP 1


using namespace std;

bool isPrime(int number)
{   for(int i = 2; i <= number/2; ++i)
    {
        if(number % i == 0)
        {
            return false;
        }
    }
    return true;
};
int generateRandomNumber(int current_number)
{   
    if (current_number <=0 )
    {
      current_number = 1;
    }
    srand(time(NULL));
    int increasing = rand() % 100 + 1; 
    return (current_number += increasing) ;
};

int main(int argc, char *argv[])
{
    int infPipe[2];
    // Create Pipe Descriptors
    int statusPipe = pipe(infPipe);
    if (statusPipe == -1)
    {
        cout << "PIPE CREATION FAILURE" << endl;
        return EXIT_PIPE_FAILURE;
    }

    int statusFork = fork();
    if (statusFork == -1)
    {
        cout << "FORK CREATION FAILURE" << endl;
        return EXIT_FORK_FAILURE;
    }

    int numberOfProducts = atoi(argv[1]);
    int currentProduction = 0;

    // fork() returns 0 for child process, child-pid for parent process.
    if (statusFork > 0)
    {
        // Producer - Parent Process: Initalizing, so close read-descriptor.
        close(infPipe[0]);
        int interations = 0;
        char charDataMessage[20];
        while( interations < numberOfProducts )
        {
            currentProduction = generateRandomNumber(currentProduction);
            int n = sprintf(charDataMessage, "%d", currentProduction);
            cout << "Producer is about to send: " << charDataMessage << endl;
            write(infPipe[1], charDataMessage, 20);
            sleep(TIME_SLEEP);
            interations+=1;
        };
        std::ostringstream stringOut;
        stringOut << numberOfProducts;
        cout << "The Producer has produced the required number of times: "<< stringOut.str() << endl;
        write(infPipe[1], "0", 20);
        close(infPipe[1]);
        exit(0);
        
    }
        
       else 
       {
        // Consumer. This is the child process. Close other end first.
        char charMessage[20];
        while (1) {
            close(infPipe[1]);
            read(infPipe[0], charMessage, 20);
            cout << "Consumer received: " << charMessage << endl;
            int intMessage = atoi(charMessage);
            if (intMessage == 0) {
                break;
            };
            if (isPrime(intMessage)) {
                cout << "Number " << charMessage << " is prime"<< endl;
            } else {
                cout << "Number " << charMessage << " is not prime"<< endl;
            }
        }
        close(infPipe[0]);
        cout << "All products received"<< endl;
        exit(0);
        }
        return EXIT_SUCCESS;
}
