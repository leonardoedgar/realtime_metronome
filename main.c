#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

// Output color definition
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"
// Type define
typedef struct
{
    char modetemp[1024];
    int modeNum;
    int frequency;
} setting;

typedef struct
{
    int min;
    int max;
} freqLimit;

//Prototype functions
int readArrow();
freqLimit getFreqLimit();
setting getfrequency(setting userSetting);
int adjustFreq(setting userSetting);
void printTempo();
void* SpawnUserInputThread( void* arg );
void* SpawnAudioThread( void* arg );
void signal_handler( int signum );
void PrintAudio( int signum );

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid[2];
bool terminateProgram = false;
int numThreadsAlive=0;
int count = 0;
int numOfChar = 0;
timer_t timerid;
setting userSetting;
char *tempo[] = {"", "Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", "Allegro", "Vivace", "Presto", "Prestissimo"};
int BPM[] = {0, 12, 35, 53, 64, 72, 93, 133, 167, 189, 351};
int minBPM[] = {0, 0, 25, 46, 61, 67, 77, 109, 157, 177, 201};
int maxBPM[] = {0, 24, 45, 60, 66, 76, 108, 156, 176, 200, 500};

int main(void)
{
    bool exitLoop;
    pthread_create(&(tid[0]), NULL, SpawnAudioThread, NULL );
    pthread_create(&(tid[1]), NULL, SpawnUserInputThread, NULL );
    signal( SIGINT, signal_handler );
    sleep(1);
    pthread_mutex_lock( &mutex );
    exitLoop = numThreadsAlive == 0;
    pthread_mutex_unlock( &mutex );
    while(!exitLoop) {
        pthread_mutex_lock( &mutex );
        exitLoop = numThreadsAlive == 0;
        pthread_mutex_unlock( &mutex );
    }
    return 0;
}

setting getfrequency(setting userSetting)
{
    // Print out the list to choose modeNum and basical frequency
    printTempo();
    do
    {
        printf("Please choose tempo mode:");
        if (!fgets(userSetting.modetemp, 1024, stdin))
        {
            continue;
        }
        userSetting.modeNum = atoi(userSetting.modetemp);
        if (userSetting.modeNum > 9)
            userSetting.modeNum = 0;
    } while (userSetting.modeNum == 0);
    printf("Your mode: %d, Corresponding BPM: %d\n", userSetting.modeNum, userSetting.frequency = BPM[userSetting.modeNum]);
    return userSetting;
}

int readArrow()
{
    // Read the up and down arrow key to adjust frequency
    int int_1 = 0;
    int int_2 = 0;
    int int_3 = 0;

    printf("Please using up and down key to adjust the frequency you want.\n");
    system("/bin/stty raw");
    scanf("%d", &int_3);
    int_1 = getchar();
    if (int_1 == 27)
    {
        int_2 = getchar();
        int_3 = getchar();
        printf("\r          ");
    }
    system("/bin/stty cooked");

    return int_3;
}

freqLimit getFreqLimit(int modeNum)
{

    freqLimit frequencyRange;

    switch (modeNum)
    {
    case 1:
        frequencyRange.min = 0;
        frequencyRange.max = 24;
        break;
    case 2:
        frequencyRange.min = 25;
        frequencyRange.max = 45;
        break;
    case 3:
        frequencyRange.min = 46;
        frequencyRange.max = 60;
        break;
    case 4:
        frequencyRange.min = 61;
        frequencyRange.max = 66;
        break;
    case 5:
        frequencyRange.min = 67;
        frequencyRange.max = 76;
        break;
    case 6:
        frequencyRange.min = 77;
        frequencyRange.max = 108;
        break;
    case 7:
        frequencyRange.min = 109;
        frequencyRange.max = 156;
        break;
    case 8:
        frequencyRange.min = 157;
        frequencyRange.max = 176;
        break;
    case 9:
        frequencyRange.min = 177;
        frequencyRange.max = 200;
        break;
    case 10:
        frequencyRange.min = 201;
        frequencyRange.max = 500;
        break;
    }
    return frequencyRange;
}

int adjustFreq(setting userSetting)
{
    int input;
    bool exitLoop;
    pthread_mutex_lock( &mutex );
    exitLoop = terminateProgram;
    pthread_mutex_unlock( &mutex );
    freqLimit frequencyRange;
    frequencyRange = getFreqLimit(userSetting.modeNum);
    while (!exitLoop)
    {
        input = readArrow();
        switch (input)
        {
        case 65:
            printf("\n");
            if (userSetting.frequency < frequencyRange.max)
            {
                userSetting.frequency++;
                struct itimerspec timerInfo;
                int rtn;
                timerInfo.it_value.tv_sec= 60/userSetting.frequency;
                timerInfo.it_value.tv_nsec = 0;
                timerInfo.it_interval.tv_sec = 60/userSetting.frequency;
                timerInfo.it_interval.tv_nsec = 0;
                rtn = timer_settime(timerid, 0, &timerInfo, NULL);
                if( rtn == -1 ) {
                    printf("\nError setting timer!\n");
                    exit(1);
                }
            }
            else
            {
                printf(KYEL "[WARN]   Maximum limit reached, can't add more.\n" KNRM);
            }
            break;
        case 66:
            printf("\n");
            if (frequencyRange.min < userSetting.frequency)
            {
                userSetting.frequency--;
                struct itimerspec timerInfo;
                int rtn;
                timerInfo.it_value.tv_sec= 60/userSetting.frequency;
                timerInfo.it_value.tv_nsec = 0;
                timerInfo.it_interval.tv_sec = 60/userSetting.frequency;
                timerInfo.it_interval.tv_nsec = 0;
                rtn = timer_settime(timerid, 0, &timerInfo, NULL);
                if( rtn == -1 ) {
                    printf("\nError setting timer!\n");
                    exit(1);
                }
            }
            else
            {
                printf(KYEL "[WARN]   Minimum limit reached, can't reduce more.\n" KNRM);
            }
            break;
        case 67:
            printf("Right!\n");
            break;
        case 68:
            printf("Left!\n");
            break;
        case 'q':
            pthread_mutex_lock( &mutex );
            terminateProgram = true;
            pthread_mutex_unlock( &mutex );
            break;
        default:
            pthread_mutex_lock( &mutex );
            terminateProgram = true;
            pthread_mutex_unlock( &mutex );
            printf("end editing\n");
            break;
        }
        printf("The current frequency is %d.\n", userSetting.frequency);
        fflush(stdout);
        pthread_mutex_lock( &mutex );
        exitLoop = terminateProgram;
        pthread_mutex_unlock( &mutex );
    }

    return userSetting.frequency;
}

void printTempo()
{
    int i;
    printf("List of tempo\n");
    printf("===================================================================\n");
    printf("\tMode  \t\tTempo\t\tBPM\t\tRange\n");
    printf("===================================================================\n");
    for (i = 1; i <= 10; i++)
    {
        printf("\t%2d\t%14s\t\t%3d\t     %3d ~ %3d\n", i, tempo[i], BPM[i], minBPM[i], maxBPM[i]);
    }
}

void* SpawnUserInputThread( void* arg ) {
    pthread_mutex_lock( &mutex );
    numThreadsAlive++;
    pthread_mutex_unlock( &mutex );
    printf("Start user input thread\n");
    userSetting = getfrequency(userSetting);
    struct itimerspec timerInfo;
    int rtn;
    timerInfo.it_value.tv_sec= 60/userSetting.frequency;
    timerInfo.it_value.tv_nsec = 0;
    timerInfo.it_interval.tv_sec = 60/userSetting.frequency;
    timerInfo.it_interval.tv_nsec = 0;
    rtn = timer_settime(timerid, 0, &timerInfo, NULL);
    if( rtn == -1 ) {
        printf( "\nError setting timer!\n" ); exit(1);
    }
    userSetting.frequency = adjustFreq(userSetting);
    printf("End user input thread\n");
    pthread_mutex_lock( &mutex );
    numThreadsAlive--;
    pthread_mutex_unlock( &mutex );
    pthread_exit(NULL);
}

void* SpawnAudioThread( void* arg ) {
    bool exitLoop;
    pthread_mutex_lock( &mutex );
    numThreadsAlive++;
    exitLoop = terminateProgram;
    pthread_mutex_unlock( &mutex );
    printf("Start audio thread\n");
    signal( SIGALRM, PrintAudio ); // Set handling for exceptional conditions
    if (timer_create( CLOCK_REALTIME, NULL, &timerid ) == -1 ) {
        printf( "Error: Failed to create timer\n" );
        exit( -1 );
    }
    while(!exitLoop) {
        pthread_mutex_lock( &mutex );
        exitLoop = terminateProgram;
        pthread_mutex_unlock( &mutex );
    }
    printf("End audio thread\n");
    timer_delete(timerid);
    pthread_mutex_lock( &mutex );
    numThreadsAlive--;
    pthread_mutex_unlock( &mutex );
    pthread_exit(NULL);
}

void signal_handler( int signum ) {
    printf( "Ctrl-C raised.\n" );
    pthread_mutex_lock( &mutex );
    terminateProgram = true;
    pthread_mutex_unlock( &mutex );
}

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
