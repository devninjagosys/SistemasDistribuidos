#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <atomic>
#include <vector>
#include <thread>
// #include "Somador.h"

using namespace std;


//struct Lock {
//    bool held;
//};

void acquire(std::atomic_flag* lock){
    while (lock->test_and_set()) ;
}

void release(std::atomic_flag* lock){
    lock->clear();
}



// Global variables:
int64_t totalSum = 0;
std::atomic_flag sumLock = ATOMIC_FLAG_INIT;
// ------------------



void vectorSum(std::vector<int8_t> v){
    // Sums the values in the vector slice given
    int64_t sumOfValues = 0;

    for(std::vector<int8_t>::iterator it = v.begin(); it != v.end(); ++it)
        sumOfValues += *it;

    // Write value to the memory -- CRITICAL REGION
    acquire(&sumLock);
    totalSum = totalSum + sumOfValues;
    release(&sumLock);
    // End of critial region
}



int main(int argc, char* argv[]){
    if (argc < 3){
        cout << "ERROR: Not enough arguments." << endl;
        cout << "       This program needs to be given the number of values to "
             << "be summed and the number of threads to do so." << endl;
        cout << "       Also can be given the name of output file. Default set "
             << "to 'sumLatencies'" << endl;
        cout << "       >>> ./main N_VALUES N_THREADS [OUTPUT_NAME]" << endl;
        exit(1);
    }

    int64_t N;
    int K = atoi(argv[2]);
    string fileRoot;

    std::istringstream iss(argv[1]);
    iss >> N;

    if (argc > 3) fileRoot = argv[3];
    else fileRoot = "sumLatencies";

    string filename = fileRoot + ".txt";

    cout << N << " values to sum. " << K << " threads." << endl;

    sumLock.clear();

    // Creation and filling of the vector of numbers
    std::vector<int8_t> list;
    srand(time(NULL));
    for (int i=0; i<N; i++){
        int tmp = rand() % 201 - 100;
        list.push_back( tmp );
        // cout << "Number " << i+1 << ": " << tmp << endl;
    }

    // Execution of the sum itself
    cout << "Will start to calculate the sum now" << endl;
    clock_t t_0 = clock();

    std::vector<std::thread> threadsVec;
    int numbPerThr = int(N/K);

    vector<int8_t>::iterator begin = list.begin();
    vector<int8_t>::iterator last = begin + numbPerThr;

    for (int thrIdx; thrIdx<K; thrIdx++){
        // cout << "Thread " << thrIdx + 1 << ", begining in " <<
        //     begin - list.begin() + 1 << " and finishing in "
        //     << last - list.begin() << endl;
        threadsVec.push_back( std::thread( vectorSum, std::vector<int8_t>(begin, last) ) );

        begin = last;
        last = begin + numbPerThr;
        if (thrIdx == K - 2){
            last = list.end();
        }
    }

    for (int thrIdx; thrIdx<K; thrIdx++){
        threadsVec[thrIdx].join();
    }
    clock_t t = clock();
    double totTime = (float)(t - t_0)/CLOCKS_PER_SEC;
    cout << "Finished all threads. Total sum is " << totalSum << " Time to calculate sum: " << totTime << "s." << endl;

    // Will write data to file
    cout << "Writing to file '" << filename << "'" << endl;
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app);
    outfile << N << " " << K << " " << totTime << " " << endl;
    outfile.close();
    cout << "Done. Finishing program." << endl;

    return 0;
}