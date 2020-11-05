#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int count = 0;
int numOfChar = 0;

void PrintAudio( int signum ) {
    if (count >= 4) {
        int i;
        printf("\r");
        for(i=0; i<numOfChar; i++) {
            printf(" ");
        }
        printf("\r");
        numOfChar = 0;
        count = 0;
    }
    if(count%2) {
        printf("bob ");
        fflush(stdout);
        numOfChar += 4;
    }
    else {
        printf("beep ");
        fflush(stdout);
        numOfChar += 5;
    }
    ++count;
}

int main() {
    timer_t timerid;
    struct itimerspec timerInfo;
    int rtn;
    signal( SIGALRM, PrintAudio ); // Set handling for exceptional conditions
    if (timer_create( CLOCK_REALTIME, NULL, &timerid ) == -1 ) {
        printf( "Error: Failed to create timer\n" );
        exit( -1 );
    }
    timerInfo.it_value.tv_sec= 1;
    timerInfo.it_value.tv_nsec = 0;
    timerInfo.it_interval.tv_sec = 1;
    timerInfo.it_interval.tv_nsec = 0;
    rtn = timer_settime(timerid, 0, &timerInfo, NULL);
    if( rtn == -1 ) {
        printf( "\nError setting timer!\n" ); exit(1); }
    while(1) {
        sleep(1);
    }
    timer_delete(timerid);
    return 0;
}