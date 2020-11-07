#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <math.h>
#include <string.h>

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
#define PLATFORM_ID QNX_ID

#if PLATFORM_ID == QNX_ID
#define TERMINAL_WIDTH QNX_TERMINAL_WIDTH
#else
#define TERMINAL_WIDTH UBUNTU_TERMINAL_WIDTH
#endif

/**
 * A data type that contains the metronome setting.
 */
typedef struct {
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

int ReadArrow();
freqLimit GetFreqLimit();
setting GetFrequency(setting metronomeSetting);
int AdjustFreq(setting metronomeSetting);
void PrintTempo();

/**
 * A function to spawn the user input thread.
 * @param arg contains the argument to be passed to the thread
 * @return NULL
 */
void* SpawnUserInputThread(void* defaultSetting);

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

/**
 * A function to parse the command-line arguments.
 * @param argc represents the number of arguments
 * @param argv represents contains an array of arguments
 * @param filePath represents the file path passed as a argument
 */
void ParseArgs(int argc, char** argv, char** filePath);

/**
 * A function to parse the config file.
 * @param filePath indicates the path to the config file
 * @param metronomeSetting indicates the metronome setting written in the file
 */
void ParseConfigFile(char* filePath, setting* metronomeSetting);

/**
 * A function to check if a metronome mode number is valid.
 * @param modeNum indicates the mode number
 * @return true if valid, otherwise false
 */
bool IsMetronomeModeNumValid(int modeNum);

/**
 * A function to check if a metronome frequency valid within its mode.
 * @param modeNum indicates the mode number
 * @param frequency indicates the frequency
 * @return true if valid, otherwise false
 */
bool IsMetronomeFrequencyValid(int modeNum, int frequency);

/**
 * A function to save the metronome setting to a file.
 * @param filePath indicates the path to the file to save the settting to
 * @param metronomeSetting indicates the metronome setting to be saved
 * @return true for a successful saving, otherwise false
 */
bool SaveMetronomeSetting(char* filePath, setting* metronomeSetting);


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid[2];
bool terminateProgram = false;
int numThreadsAlive=0;
int count = 0;
timer_t timerid;
char *tempo[] = {"", "Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", "Allegro", "Vivace", "Presto", "Prestissimo"};
int BPM[] = {0, 12, 35, 53, 64, 72, 93, 133, 167, 189, 351};
int minBPM[] = {0, 1, 25, 46, 61, 67, 77, 109, 157, 177, 201};
int maxBPM[] = {0, 24, 45, 60, 66, 76, 108, 156, 176, 200, 500};

int main(int argc, char* argv[]) {
    setting metronomeSetting;
    char *configFilePath = NULL;
    bool exitProgram;
    metronomeSetting.modeNum=0;
    metronomeSetting.frequency=0;
    ParseArgs(argc, argv, &configFilePath);
    if (configFilePath != NULL) {
        ParseConfigFile(configFilePath, &metronomeSetting);
    }
    signal(SIGINT, CtrlCHandler);
    pthread_mutex_lock(&mutex);
    exitProgram = terminateProgram;
    pthread_mutex_unlock(&mutex);
    if (!exitProgram) {
        pthread_create(&(tid[0]), NULL, SpawnAudioThread, NULL);
        pthread_create(&(tid[1]), NULL, SpawnUserInputThread, (void *)&metronomeSetting);
        sleep(1);
        pthread_mutex_lock(&mutex);
        exitProgram = numThreadsAlive == 0;
        pthread_mutex_unlock(&mutex);
        while(!exitProgram) {
            pthread_mutex_lock(&mutex);
            exitProgram = numThreadsAlive == 0;
            pthread_mutex_unlock(&mutex);
        }
        if(configFilePath != NULL) {
            if(!SaveMetronomeSetting(configFilePath, &metronomeSetting)) {
                printf("Failed to save the latest metronome setting into the file: %s\n", configFilePath);
            }
        }
    }
    free(configFilePath);
    return 0;
}

void PrintTempo() {
    int i;
    printf("                            LIST OF TEMPO\n");
    printf("===================================================================\n");
    printf("\tMode  \t\tTempo\t\tBPM\t\tRange\n");
    printf("===================================================================\n");
    for (i = 1; i <= 10; i++) {
        printf("\t%2d\t%14s\t\t%3d\t     %3d ~ %3d\n", i, tempo[i], BPM[i], minBPM[i], maxBPM[i]);
    }
}

setting GetFrequency(setting metronomeSetting) {
    bool exitLoop = metronomeSetting.modeNum != 0;
    char modeStr[1024];
    do {
        printf("Choose tempo mode: ");
        if (!fgets(modeStr, 1024, stdin)) {
            printf("data: %s\n", modeStr);
        }
        else if (fabs(atoi(modeStr) - atof(modeStr)) > NEARLY_ZERO) {
            printf("Input is a floating point number.\n");
        }
        else if (!IsMetronomeModeNumValid(atoi(modeStr))) {}
        else {
            metronomeSetting.modeNum = atoi(modeStr);
        }
        pthread_mutex_lock( &mutex );
        exitLoop = terminateProgram;
        pthread_mutex_unlock( &mutex );
    } while (metronomeSetting.modeNum == 0 && !exitLoop);
    metronomeSetting.frequency = BPM[metronomeSetting.modeNum];
    return metronomeSetting;
}

int ReadArrow() {
    // Read the up and down arrow key to adjust frequency
    int int_1 = 0;
    int int_3 = 0;
    system("/bin/stty raw");
    scanf("%d", &int_3);
    int_1 = getchar();
    if (int_1 == 27) {
        getchar();
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

freqLimit GetFreqLimit(int modeNum) {
    freqLimit frequencyRange;
    frequencyRange.min = minBPM[modeNum];
    frequencyRange.max = maxBPM[modeNum];
    return frequencyRange;
}

int AdjustFreq(setting metronomeSetting) {
    int input;
    freqLimit frequencyRange;
    bool exitLoop;
    pthread_mutex_lock(&mutex);
    exitLoop = terminateProgram;
    pthread_mutex_unlock(&mutex);
    frequencyRange = GetFreqLimit(metronomeSetting.modeNum);
    printf("Please use arrow up and down key to adjust the frequency you want.\n\n");
    printf("Audio: ");
    while (!exitLoop) {
        input = ReadArrow();
        switch (input) {
        case 65:
            if (metronomeSetting.frequency < frequencyRange.max) {
                metronomeSetting.frequency++;
                PrintAdjustFreqInstructions(metronomeSetting.modeNum, metronomeSetting.frequency, NULL);
                if(UpdateTimer(metronomeSetting.frequency) == -1 ) {
                    printf("\nError setting timer!\n");
                    TerminateProgram();
                }
            }
            else {
                PrintAdjustFreqInstructions(metronomeSetting.modeNum, metronomeSetting.frequency,
                        "Maximum limit reached, unable to increase further.");
            }
            break;
        case 66:
            if (frequencyRange.min < metronomeSetting.frequency) {
                metronomeSetting.frequency--;
                PrintAdjustFreqInstructions(metronomeSetting.modeNum, metronomeSetting.frequency, NULL);
                if(UpdateTimer(metronomeSetting.frequency) == -1) {
                    printf("\nError setting timer!\n");
                    TerminateProgram();
                }
            }
            else {
                PrintAdjustFreqInstructions(metronomeSetting.modeNum, metronomeSetting.frequency,
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
    return metronomeSetting.frequency;
}

void* SpawnUserInputThread(void* defaultSetting) {
    setting metronomeSetting = *((setting*)defaultSetting);
    pthread_mutex_lock(&mutex);
    numThreadsAlive++;
    pthread_mutex_unlock(&mutex);
    printf("Start user input thread\n");
    PrintTempo();
    if(metronomeSetting.modeNum == 0 || metronomeSetting.frequency == 0) {
        metronomeSetting = GetFrequency(metronomeSetting);
    }
    if(UpdateTimer(metronomeSetting.frequency) == -1) {
        printf( "\nError setting timer!\n" );
        TerminateProgram();
    }
    printf("Your tempo: %s, corresponding BPM: %d\n", tempo[metronomeSetting.modeNum], metronomeSetting.frequency);
    metronomeSetting.frequency = AdjustFreq(metronomeSetting);
    ((setting*)defaultSetting)->frequency = metronomeSetting.frequency;
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
    printf("\r");
    for(i=0; i<TERMINAL_WIDTH; i++) {
        printf(" ");
    }
    printf("\rYour tempo: %s, corresponding BPM: %d\n", tempo[mode], frequency);
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

void ParseArgs(int argc, char** argv, char** filePath) {
    int i;
    if (argc > 1) {
        for (i=1; i<argc; i++) {
            if (strcmp(*(argv+i), "--config") == 0) {
                *filePath = malloc(strlen(*(argv+i+1)) + 1);
                strcpy(*filePath, *(argv+i+1));
                break;
            }
        }
    }
}

void ParseConfigFile(char* filePath, setting* metronomeSetting) {
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool isModeConfigFound = false, isFrequencyConfigFound = false;
    stream = fopen(filePath, "r");
    if (stream == NULL) {
        printf("Err: file with path %s not found. \n", filePath);

    }
    else {
        while ((read = getline(&line, &len, stream)) != -1) {
            if (strstr(line, "MODE")) {
                line = strstr(line, "=");
                metronomeSetting->modeNum = atoi(&line[1]);
                isModeConfigFound = true;
            }
            else if (strstr(line, "FREQUENCY")) {
                line = strstr(line, "=");
                metronomeSetting->frequency = atoi(&line[1]);
                isFrequencyConfigFound = true;
            }
        }
        fclose(stream);
    }
    if (!isModeConfigFound || !isFrequencyConfigFound) {
        printf("Configuration file format error.\n");
        TerminateProgram();
    }
    else if (!IsMetronomeModeNumValid(metronomeSetting->modeNum)) {
        TerminateProgram();
    }
    else {
        if (!IsMetronomeFrequencyValid(metronomeSetting->modeNum, metronomeSetting->frequency)) {
            TerminateProgram();
        }
    }
}

bool IsMetronomeModeNumValid(int modeNum) {
    if (modeNum <=10 && modeNum >= 1) {
        return true;
    }
    else {
        printf("Setting with mode: %d is not within the valid range (1-10).\n", modeNum);
        return false;
    }
}

bool IsMetronomeFrequencyValid(int modeNum, int frequency) {
    freqLimit frequencyRange;
    if (IsMetronomeModeNumValid(modeNum)) {
        frequencyRange = GetFreqLimit(modeNum);
        if (frequency <= frequencyRange.max && frequency >= frequencyRange.min) {
            return true;
        }
        else {
            printf("Setting with tempo: %s, with frequency: %d is not within the valid range (%d-%d).\n",
                   tempo[modeNum], frequency, frequencyRange.min, frequencyRange.max);
        }
    }
    return false;
}

bool SaveMetronomeSetting(char* filePath, setting* metronomeSetting) {
    FILE* stream;
    bool success;
    stream = fopen(filePath, "w");
    success = fprintf(stream, "MODE=%d\nFREQUENCY=%d\n", metronomeSetting->modeNum, metronomeSetting->frequency) > 0;
    fclose(stream);
    return success;
}
