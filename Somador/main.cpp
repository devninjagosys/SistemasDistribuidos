#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <atomic>
#include <vector>
#include <thread>
// #include "Somador.h"

using namespace std;


//struct Lock {
//    bool held;
//};

void acquire(std::atomic_flag lock){
    while (lock.test_and_set()) ;
}

void release(std::atomic_flag lock){
    lock.clear();
}



// Global variables:
int64_t totalSum = 0;
std::atomic_flag sumLock = ATOMIC_FLAG_INIT;
//Lock sumLock;
// ------------------



void vectorSum(std::vector<int8_t> v){
    // Sums the values in the vector slice given
    int64_t sumOfValues = 0;

    for(std::vector<int8_t>::iterator it = v.begin(); it != v.end(); ++it)
        sumOfValues += *it;

    // cout << "Got sum " << sumOfValues << endl;
}



int main(int argc, char* argv[]){
    // TODO: What arguments should be given to the code?
    // Could choose to select the N and K, or test all values of K and N...
    int     K=1;
    int64_t N=1e9;

    cout << N << " values to sum. " << K << " threads." << endl;


    // Creation and filling of the vector of numbers
    std::vector<int8_t> list;
    srand(time(NULL));
    for (int i=0; i<N; i++){
        int tmp = rand() % 201 - 100;
        list.push_back( tmp );
        // cout << "Number " << i+1 << ": " << tmp << endl;
    }

    // Execution of the sum itself
    clock_t t_0 = clock();

    std::vector<std::thread> threadsVec;
    int numbPerThr = int(N/K);

    vector<int8_t>::iterator begin = list.begin();
    vector<int8_t>::iterator last = begin + numbPerThr;

    for (int thrIdx; thrIdx<K; thrIdx++){
        cout << "Thread " << thrIdx + 1 << ", begining in " <<
            begin - list.begin() + 1 << " and finishing in "
            << last - list.begin() << endl;

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
    cout << "Finished all threads. Time to calculate sum: " << (float)t/CLOCKS_PER_SEC << "s." << endl;
}