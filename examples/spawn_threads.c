#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t tid[2];

bool terminateProgram = false;
int numThreadsAlive=0;

void* thds1( void* arg ) {
    int i=0;
    pthread_mutex_lock( &mutex );
    numThreadsAlive++;
    pthread_mutex_unlock( &mutex );
    printf("Start thread 1\n");
    while(!terminateProgram) {
        printf("Thread 1: %d ", i);
        fflush(stdout);
        sleep(1);
        i++;
    }
    printf("End thread 1\n");
    numThreadsAlive--;
    pthread_exit(NULL);
}

void* thds2( void* arg ) {
    int i=0;
    pthread_mutex_lock( &mutex );
    numThreadsAlive++;
    pthread_mutex_unlock( &mutex );
    printf("Start thread 2\n");
    while(!terminateProgram) {
        printf("thread 2: %d ", i);
        fflush(stdout);
        sleep(1);
        i++;
    }
    printf("End thread 2\n");
    numThreadsAlive--;
    pthread_exit(NULL);
}

void signal_handler( int signum ) {
    printf( "Ctrl-C raised.\n" );
    terminateProgram = true;
}

int main() {
    pthread_create(&(tid[0]), NULL, thds1, NULL );
    pthread_create(&(tid[1]), NULL, thds2, NULL );
    signal( SIGINT, signal_handler );
    sleep(1);
    while(1) {
        pthread_mutex_lock( &mutex );
        if (numThreadsAlive == 0) {
            break;
        }
        pthread_mutex_unlock( &mutex );
    }
    return 0;
}