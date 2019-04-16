#include <iostream>      
#include <thread>
#include <vector>
#include <mutex>         
#include <condition_variable>

using namespace std;

std::mutex mtx;             // mutex for critical section
std::condition_variable cv; // condition variable for critical section  
bool ready = false;         // Tell threads to run
int current = 0;            // current count

int numberOfProducts = 10000;
int sharedMemorySize = 32;
vector<int> sharedMemory(sharedMemorySize);
vector<int> consumerLocalMemory(numberOfProducts);

int numberOfProductsProduced = 0;
int numberOfProductsConsumed = 0;

//Some variable needed by the semaphore
mutex semaphoreMutex;
condition_variable semaphoreFull;
condition_variable semaphoreEmpty;


void bufferIsFull;
void bufferIsEmpty;
void consumer;
void producer;
void getFirstFreePosition;
void getFirstFullPosition;

int isPrime(int number)
{   for(int i = 2; i <= number/2; ++i)
    {
    if(number % i == 0)
        {
          return 0;
        }
    }
    return 1;
}

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

void print_num(int num, int max) {

  std::unique_lock<std::mutex> lck(mtx);
  while(num != current || !ready){ cv.wait(lck); }
  current++;
  std::cout << "Thread: ";
  std::cout << num + 1 << " / " << max;
  std::cout << " current count is: ";
  std::cout << current << std::endl;
  
  /* Notify next threads to check if it is their turn */
  cv.notify_all(); 
}

/* Changes ready to true, and begins the threads printing */
void run(){
  std::unique_lock<std::mutex> lck(mtx);
  ready = true;
  cv.notify_all();
}
 
int main (){

  int threadnum = 15;
  std::thread threads[15];

  /* spawn threadnum threads */
  for (int id = 0; id < threadnum; id++)
    threads[id] = std::thread(print_num, id, threadnum);

  std::cout << "\nRunning " << threadnum;
  std::cout << " in parallel: \n" << std::endl;

  run(); // Allows threads to run

  /* Merge all threads to the main thread */
  for(int id = 0; id < threadnum; id++)
    threads[id].join();

  std::cout << "\nCompleted semaphore example!\n";
  std::cout << std::endl;

  return 0;
}