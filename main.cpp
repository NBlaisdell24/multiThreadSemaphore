//
// Example from: http://www.amparo.net/ce155/sem-ex.c
//
// Adapted using some code from Downey's book on semaphores
//
// Compilat//       g++ main.cpp -lpthread -o cse4001_sync -lm
// or 
//      make
//

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#include <iostream>
using namespace std;

/*
 This wrapper class for semaphore.h functions is from:
 http://stackoverflow.com/questions/2899604/using-sem-t-in-a-qt-project
 */
class Semaphore {
public:
    // Constructor
    Semaphore(int initialValue)
    {
        sem_init(&mSemaphore, 0, initialValue);
    }
    // Destructor
    ~Semaphore()
    {
        sem_destroy(&mSemaphore); /* destroy semaphore */
    }
    
    // wait
    void wait()
    {
        sem_wait(&mSemaphore);
    }
    // signal
    void signal()
    {
        sem_post(&mSemaphore);
    }
    
    
private:
    sem_t mSemaphore;
};

// Lightswitch class
class Lightswitch {
public:
    Lightswitch() : counter(0), mutex(1) {}
    
    void lock(Semaphore& semaphore) {
        mutex.wait();
        counter++;
        if (counter == 1) {
            semaphore.wait();
        }
        mutex.signal();
    }
    
    void unlock(Semaphore& semaphore) {
        mutex.wait();
        counter--;
        if (counter == 0) {
            semaphore.signal();
        }
        mutex.signal();
    }
    
private:
    int counter;
    Semaphore mutex;
};



/* global vars */
const int bufferSize = 5;
const int numConsumers = 3; 
const int numProducers = 3; 

/* semaphores are declared global so they can be accessed
 in main() and in thread routine. */
Semaphore Mutex(1);
Semaphore Spaces(bufferSize);
Semaphore Items(0);             
//Reader Writer no starve semaphores
Semaphore turnstile(1);
Semaphore roomEmpty(1);
Lightswitch readSwitch;

// Writer-priority readers-writers semaphores
Semaphore noReaders(1);
Semaphore noWriters(1);
Lightswitch writeSwitch;



// Dining philosophers semaphores
const int numPhilosophers = 5;
Semaphore forks[numPhilosophers] = {Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1)};
Semaphore footman(numPhilosophers - 1); 


int left(int i) { return i; }
int right(int i) { return (i + 1) % numPhilosophers; }

void think() {
    sleep(2);
}

void eat() {
    sleep(2);
}

void get_forks(int i) {
    footman.wait();
    forks[right(i)].wait();
    forks[left(i)].wait();
}

void put_forks(int i) {
    forks[right(i)].signal();
    forks[left(i)].signal();
    footman.signal();
}
void get_forks2(int i) {
    if (i % 2 == 0) {
        // Even philosopher
        forks[left(i)].wait();
        forks[right(i)].wait();
    } else {
        // Odd philosopher
        forks[right(i)].wait();
        forks[left(i)].wait();
    }
}
void put_forks2(int i) {
    forks[left(i)].signal();
    forks[right(i)].signal();
}
/*
    Producer function 
*/
void *Reader (void *threadID )
{
   int x = (long)threadID;

   while( 1 )
   {
       sleep(1);
       turnstile.wait();
       turnstile.signal();
       readSwitch.lock(roomEmpty);
            //Critical Section
            printf("Reader %d Reading\n", x);
            fflush(stdout);
            sleep(1);
       readSwitch.unlock(roomEmpty);
       sleep(2);
   }
}
void *WriterPriorityReader (void *threadID)
{
  int x = (long)threadID;

   while ( 1 )
   {
       noReaders.wait();
          readSwitch.lock(noWriters);
       noReaders.signal();
          //Critical Section
          printf("Reader %d Reading\n", x);
          fflush(stdout);
       readSwitch.unlock(noWriters);
   }
}

void *Producer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;

    while( 1 )
    {
        sleep(3); // Slow the thread down a bit so we can see what is going on
        Spaces.wait();
        Mutex.wait();
            printf("Producer %d adding item to buffer \n", x);
            fflush(stdout);
        Mutex.signal();
        Items.signal();
    }

}

void *Writer ( void *threadID )
{

    int x = (long)threadID;

    while( 1 )
    {
        sleep(3);
        turnstile.wait();
           roomEmpty.wait();
           //Critical Section
           printf("Writer %d Writing\n", x);
           fflush(stdout);
           sleep(1);  
        turnstile.signal();
        roomEmpty.signal();
        sleep(2);
    }
}
void *WriterPriorityWriter ( void *threadID )
{
    int x = (long)threadID;

    while( 1 )
    {
        writeSwitch.lock(noReaders);
           noWriters.wait();
              //Critical Section
                printf("Writer %d Writing\n", x);
                fflush(stdout);
           noWriters.signal();
        writeSwitch.unlock(noReaders);
    }
}

/*
    Consumer function 
*/
void *Consumer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;
    
    while( 1 )
    {
        Items.wait();
        Mutex.wait();
            printf("Consumer %d removing item from buffer \n", x);
            fflush(stdout);
        Mutex.signal();
        Spaces.signal();
        sleep(5);   // Slow the thread down a bit so we can see what is going on
    }

}

void *Philosophers (void *threadID )
{
   int i = (long)threadID;  
   
   while( 1 )
   {
       printf("Philosopher %d Thinking\n", i+1);
       fflush(stdout);
       think();
       
       printf("Philosopher %d Hungry\n", i+1);
       fflush(stdout);
       get_forks(i);
       
       printf("Philosopher %d Eating\n", i+1);
       fflush(stdout);
       eat();
       
       printf("Philosopher %d Finished eating\n", i+1);
       fflush(stdout);
       put_forks(i);
   }
}
void *Philosophers2 (void *threadID )
{
   int i = (long)threadID;  
   
   while( 1 )
   {
       printf("Philosopher %d thinking\n", i+1);
       fflush(stdout);
       think();
       
       printf("Philosopher %d hungry\n", i+1);
       fflush(stdout);
       get_forks2(i);
       
       printf("Philosopher %d eating\n", i+1);
       fflush(stdout);
       eat();
       
       printf("Philosopher %d finished eating\n", i+1);
       fflush(stdout);
       put_forks2(i);
   }
}
int main(int argc, char **argv )
{
    int choice;
    
    // Check command-line arguments
    if (argc != 2) {
        printf("Usage: %s <problem #>\n", argv[0]);
        printf("Problems:\n");
        printf("  1. Producer-Consumer\n");
        printf("  2. No-starve Readers-Writers\n");
        printf("  3. Writer-priority Readers-Writers\n");
        printf("  4. Dining Philosophers\n");
        printf("  5. Dining Philosophers solution 2\n");
        exit(1);
    }
    
    choice = atoi(argv[1]);
    
    if (choice < 1 || choice > 5) {
        printf("Invalid problem number. Please choose 1-5.\n");
        exit(1);
    }
    
    const int numReaders = 5;
    const int numWriters = 5;
    
    switch(choice) {
        case 1: {
            pthread_t producerThread[ numProducers ];
            pthread_t consumerThread[ numConsumers ];

            // Create the producers 
            for( long p = 0; p < numProducers; p++ )
            {
                int rc = pthread_create ( &producerThread[ p ], NULL, 
                                          Producer, (void *) (p+1) );
                if (rc) {
                    printf("ERROR creating producer thread # %ld; \
                            return code from pthread_create() is %d\n", p, rc);
                    exit(-1);
                }
            }

            // Create the consumers 
            for( long c = 0; c < numConsumers; c++ )
            {
                int rc = pthread_create ( &consumerThread[ c ], NULL, 
                                          Consumer, (void *) (c+1) );
                if (rc) {
                    printf("ERROR creating consumer thread # %ld; \
                            return code from pthread_create() is %d\n", c, rc);
                    exit(-1);
                }
            }
            break;
        }
        case 2: {
            pthread_t readerThread[numReaders];
            pthread_t writerThread[numWriters];
            
            
            // Create readers
            for(long r = 0; r < numReaders; r++) {
                int rc = pthread_create(&readerThread[r], NULL, Reader, (void *)(r+1));
                if (rc) {
                    printf("ERROR creating reader thread # %ld\n", r);
                    exit(-1);
                }
            }
            
            // Create writers
            for(long w = 0; w < numWriters; w++) {
                int rc = pthread_create(&writerThread[w], NULL, Writer, (void *)(w+1));
                if (rc) {
                    printf("ERROR creating writer thread # %ld\n", w);
                    exit(-1);
                }
            }
            break;
        }
        case 3: {
            pthread_t readerThread[numReaders];
            pthread_t writerThread[numWriters];
            
            
            // Create readers
            for(long r = 0; r < numReaders; r++) {
                int rc = pthread_create(&readerThread[r], NULL, WriterPriorityReader, (void *)(r+1));
                if (rc) {
                    printf("ERROR creating reader thread # %ld\n", r);
                    exit(-1);
                }
            }
            
            // Create writers
            for(long w = 0; w < numWriters; w++) {
                int rc = pthread_create(&writerThread[w], NULL, WriterPriorityWriter, (void *)(w+1));
                if (rc) {
                    printf("ERROR creating writer thread # %ld\n", w);
                    exit(-1);
                }
            }
            break;
        }
        case 4: {
            pthread_t philosopherThread[numPhilosophers];
            
            
            // Create philosophers
            for(long p = 0; p < numPhilosophers; p++) {
                int rc = pthread_create(&philosopherThread[p], NULL, Philosophers, (void *)p);
                if (rc) {
                    printf("ERROR creating philosopher thread # %ld\n", p);
                    exit(-1);
                }
            }
            break;
        }
        case 5: {
            pthread_t philosopherThread[numPhilosophers];
            
            printf("Testing Dining Philosophers Solution (Asymmetric)...\n");
            printf("This solution prevents deadlock by breaking circular wait.\n");
            printf("Even philosophers pick left fork first, odd pick right fork first.\n\n");
            
            // Create philosophers
            for(long p = 0; p < numPhilosophers; p++) {
                int rc = pthread_create(&philosopherThread[p], NULL, Philosophers2, (void *)p);
                if (rc) {
                    printf("ERROR creating philosopher thread # %ld\n", p);
                    exit(-1);
                }
            }
            break;
        }
        default:
            printf("Invalid choice.\n");
            exit(1);
    }

    pthread_exit(NULL); 

}







