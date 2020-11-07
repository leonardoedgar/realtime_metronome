#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <math.h>

/*
 * Macros definitions
 */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"

#define BILLION		1000000000L
#define NEARLY_ZERO 1e-6

#define QNX_ID 1
#define QNX_TERMINAL_WIDTH 100
#define UBUNTU_ID 2
#define UBUNTU_TERMINAL_WIDTH 195
#define PLATFORM_ID UBUNTU_ID

#if PLATFORM_ID == QNX_ID
#define TERMINAL_WIDTH QNX_TERMINAL_WIDTH
#else
#define TERMINAL_WIDTH UBUNTU_TERMINAL_WIDTH
#endif

/**
 * A data type that contains the metronome setting.
 */
typedef struct {
    char modetemp[1024];
    int modeNum;
    int frequency;
} setting;

/**
 * A data type that represents a frequency limit.
 */
typedef struct {
    int min;
    int max;
} freqLimit;

int readArrow();
freqLimit getFreqLimit();
setting getfrequency(setting userSetting);
int adjustFreq(setting userSetting);
void printTempo();

/**
 * A function to spawn the user input thread.
 * @param arg contains the argument to be passed to the thread
 * @return NULL
 */
void* SpawnUserInputThread(void* arg);

/**
 * A function to spawn the audio thread.
 * @param arg contains the argument to be passed to the thread
 * @return NULL
 */
void* SpawnAudioThread(void* arg);

/**
 * A function to handle the Ctrl-C signal.
 * @param sigNum represents the Ctrl-C signal number
 */
void CtrlCHandler(int sigNum);

/**
 * A function to print the audio signal.
 * @param sigNum represents the signal number that calls this function
 */
void PrintAudio(int sigNum);

/**
 * A function to update the timer.
 * @param frequency indicates the new frequency to be set
 * @return -1 if failed to update the timer, otherwise success
 */
int UpdateTimer(int frequency);

/**
 * A function to print instructions for the user to adjust the metronome's frequency.
 * @param mode indicates the current mode of the metronome
 * @param frequency indicates the current frequency of the metronome
 * @param warningMessage indicates the warning message to be shown to the user
 */
void PrintAdjustFreqInstructions(int mode, int frequency, const char* warningMessage);

/**
 * A function to terminate the whole program in a safe manner.
 */
void TerminateProgram();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid[2];
bool terminateProgram = false;
int numThreadsAlive=0;
int count = 0;
timer_t timerid;
setting userSetting;
char *tempo[] = {"", "Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", "Allegro", "Vivace", "Presto", "Prestissimo"};
int BPM[] = {0, 12, 35, 53, 64, 72, 93, 133, 167, 189, 351};
int minBPM[] = {0, 0, 25, 46, 61, 67, 77, 109, 157, 177, 201};
int maxBPM[] = {0, 24, 45, 60, 66, 76, 108, 156, 176, 200, 500};

int main(void)
{
    bool exitLoop;
    signal(SIGINT, CtrlCHandler);
    pthread_create(&(tid[0]), NULL, SpawnAudioThread, NULL);
    pthread_create(&(tid[1]), NULL, SpawnUserInputThread, NULL);
    sleep(1);
    pthread_mutex_lock(&mutex);
    exitLoop = numThreadsAlive == 0;
    pthread_mutex_unlock(&mutex);
    while(!exitLoop) {
        pthread_mutex_lock(&mutex);
        exitLoop = numThreadsAlive == 0;
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

void printTempo() {
    int i;
    printf("                         LIST OF TEMPO\n");
    printf("===================================================================\n");
    printf("\tMode  \t\tTempo\t\tBPM\t\tRange\n");
    printf("===================================================================\n");
    for (i = 1; i <= 10; i++) {
        printf("\t%2d\t%14s\t\t%3d\t     %3d ~ %3d\n", i, tempo[i], BPM[i], minBPM[i], maxBPM[i]);
    }
}

setting getfrequency(setting userSetting) {
	bool exitLoop = userSetting.modeNum != 0;
	printTempo();
    do {
        printf("Please choose tempo mode: ");
        if (!fgets(userSetting.modetemp, 1024, stdin)) {
            printf("data: %s\n", userSetting.modetemp);
        }
        else if (fabs(atoi(userSetting.modetemp) - atof(userSetting.modetemp)) > NEARLY_ZERO) {
            printf("Input is a floating point number.\n");
        }
        else if (atoi(userSetting.modetemp) > 10 || atoi(userSetting.modetemp) < 1) {
            printf("Setting is not within the valid range (1-10).\n");
        }
        else {
            userSetting.modeNum = atoi(userSetting.modetemp);
        }
        pthread_mutex_lock( &mutex );
        exitLoop = terminateProgram;
        pthread_mutex_unlock( &mutex );
    } while (userSetting.modeNum == 0 && !exitLoop);
    printf("Your mode: %d, Corresponding BPM: %d\n", userSetting.modeNum, userSetting.frequency = BPM[userSetting.modeNum]);
    return userSetting;
}

int readArrow()
{
    // Read the up and down arrow key to adjust frequency
    int int_1 = 0;
    int int_2 = 0;
    int int_3 = 0;
    system("/bin/stty raw");
    scanf("%d", &int_3);
    int_1 = getchar();
    if (int_1 == 27) {
        int_2 = getchar();
        int_3 = getchar();
        printf("\r          ");
    }
    if (PLATFORM_ID == QNX_ID) {
        system("/bin/stty edit");
    }
    else {
        system("/bin/stty cooked");
    }
    return int_3;
}

freqLimit getFreqLimit(int modeNum)
{
    freqLimit frequencyRange;
    switch (modeNum) {
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
    int input, i;
    freqLimit frequencyRange;
    bool exitLoop;
    pthread_mutex_lock(&mutex);
    exitLoop = terminateProgram;
    pthread_mutex_unlock(&mutex);
    frequencyRange = getFreqLimit(userSetting.modeNum);
    printf("Please use arrow up and down key to adjust the frequency you want.\n\n");
    printf("Audio: ");
    while (!exitLoop) {
        input = readArrow();
        switch (input)
        {
        case 65:
            if (userSetting.frequency < frequencyRange.max) {
                userSetting.frequency++;
                PrintAdjustFreqInstructions(userSetting.modeNum, userSetting.frequency, NULL);
                if(UpdateTimer(userSetting.frequency) == -1 ) {
                    printf("\nError setting timer!\n");
                    TerminateProgram();
                }
            }
            else {
                PrintAdjustFreqInstructions(userSetting.modeNum, userSetting.frequency,
                        "Maximum limit reached, unable to increase further.");
            }
            break;
        case 66:
            if (frequencyRange.min < userSetting.frequency) {
                userSetting.frequency--;
                PrintAdjustFreqInstructions(userSetting.modeNum, userSetting.frequency, NULL);
                if(UpdateTimer(userSetting.frequency) == -1) {
                    printf("\nError setting timer!\n");
                    TerminateProgram();
                }
            }
            else {
                PrintAdjustFreqInstructions(userSetting.modeNum, userSetting.frequency,
                        "Minimum limit reached, unable to decrease further.");
            }
            break;
        case 67:
            printf("Right!\n");
            break;
        case 68:
            printf("Left!\n");
            break;
        case 'q':
            TerminateProgram();
            break;
        default:
            TerminateProgram();
            break;
        }
        fflush(stdout);
        pthread_mutex_lock(&mutex);
        exitLoop = terminateProgram;
        pthread_mutex_unlock(&mutex);
    }
    return userSetting.frequency;
}

void* SpawnUserInputThread(void* arg) {
    pthread_mutex_lock(&mutex);
    numThreadsAlive++;
    pthread_mutex_unlock(&mutex);
    printf("Start user input thread\n");
    userSetting = getfrequency(userSetting);
    if(UpdateTimer(userSetting.frequency) == -1) {
        printf( "\nError setting timer!\n" );
        TerminateProgram();
    }
    userSetting.frequency = adjustFreq(userSetting);
    printf("End user input thread\n");
    pthread_mutex_lock(&mutex);
    numThreadsAlive--;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void* SpawnAudioThread(void* arg) {
    bool exitLoop;
    pthread_mutex_lock(&mutex);
    numThreadsAlive++;
    exitLoop = terminateProgram;
    pthread_mutex_unlock(&mutex);
    printf("Start audio thread\n");
    signal( SIGALRM, PrintAudio );
    if (timer_create( CLOCK_REALTIME, NULL, &timerid) == -1 ) {
        printf( "Error: Failed to create timer\n" );
        TerminateProgram();
    }
    while(!exitLoop) {
        pthread_mutex_lock(&mutex);
        exitLoop = terminateProgram;
        pthread_mutex_unlock(&mutex);
    }
    printf("End audio thread\n");
    timer_delete(timerid);
    pthread_mutex_lock(&mutex);
    numThreadsAlive--;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void CtrlCHandler(int sigNum) {
    printf( "Ctrl-C raised.\n" );
    TerminateProgram();
}

void PrintAudio(int sigNum) {
    if (count >= 4) {
        int i;
        printf("\r");
        printf("Audio: ");
        for(i=0; i<TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        for(i=0; i<TERMINAL_WIDTH; i++) {
            printf("\b");
        }
        count = 0;
    }
    if(count%2) {
        printf("bob ");
        putchar(7);
        fflush(stdout);
    }
    else {
        printf("beep ");
        putchar(7);
        fflush(stdout);
    }
    ++count;
}

int UpdateTimer(int frequency) {
    struct itimerspec timerInfo;
    timerInfo.it_value.tv_sec= 60/frequency;
    timerInfo.it_value.tv_nsec = ((long long)60*BILLION/frequency - (long long)timerInfo.it_value.tv_sec*BILLION);
    timerInfo.it_interval.tv_sec = 60/frequency;
    timerInfo.it_interval.tv_nsec = ((long long)60*BILLION/frequency - (long long)timerInfo.it_value.tv_sec*BILLION);
    return timer_settime(timerid, 0, &timerInfo, NULL);
}

void PrintAdjustFreqInstructions(int mode, int frequency, const char* warningMessage) {
    int i;
    printf("\033[3A");
    printf("\rYour mode: %d, Corresponding BPM: %d\n", mode, frequency);
    printf("Please use arrow up and down key to adjust the frequency you want.\n");
    if (warningMessage != NULL) {
        printf(KYEL "[WARN]   %s\n", warningMessage);
        printf(KNRM);
        printf("Audio: ");
        for(i=0; i<TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        for(i=0; i<TERMINAL_WIDTH; i++) {
            printf("\b");
        }
    }
    else {
        printf("                                                                  \n");
        for(i=0; i<TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        printf("\rAudio: ");
    }
}

void TerminateProgram() {
    pthread_mutex_lock(&mutex);
    terminateProgram = true;
    pthread_mutex_unlock(&mutex);
}
