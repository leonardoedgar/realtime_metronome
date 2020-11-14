#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "main.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool terminateProgram = false;
timer_t timerid;

int main(int argc, char* argv[]) {
    struct sigaction ctrlCAction;
    pthread_t tid[2];
    sigset_t signalSet;
    setting metronomeSetting;
    char* configFilePath = NULL;
    bool exitProgram;
    metronomeSetting.modeNum = 0;
    metronomeSetting.frequency = 0;
    ParseArgs(argc, argv, &configFilePath);
    if (configFilePath != NULL) {
        ParseConfigFile(configFilePath, &metronomeSetting);
    }
    memset(&ctrlCAction, 0, sizeof(struct sigaction));
    ctrlCAction.sa_handler = CtrlCHandler;
    ctrlCAction.sa_flags = 0;
    sigaction(SIGINT, &ctrlCAction, NULL);
    sigemptyset(&signalSet);
    sigaddset(&signalSet, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &signalSet, NULL) == -1) {
        printf(KRED "[ERROR] Failed to set signal masks.\n" KNRM);
        TerminateProgram();
    }
    pthread_mutex_lock(&mutex);
    exitProgram = terminateProgram;
    pthread_mutex_unlock(&mutex);
    if (!exitProgram) {
        pthread_create(&(tid[0]), NULL, SpawnAudioThread, NULL);
        pthread_create(&(tid[1]), NULL, SpawnUserInputThread, (void *)&metronomeSetting);
        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);
        if (configFilePath != NULL) {
            if (!SaveMetronomeSetting(configFilePath, &metronomeSetting)) {
                printf(KRED "[ERROR] Failed to save the latest metronome setting into the file: %s\n" KNRM, configFilePath);
            }
        }
    }
    if(configFilePath != NULL) {
        free(configFilePath);
    }
    return 0;
}

void PrintTempo() {
    int modeNum, bpm;
    char* tempo = "";
    freqLimit freqRange;
    printf("                            LIST OF TEMPO\n");
    printf("===================================================================\n");
    printf("\tMode  \t\tTempo\t\tBPM\t\tRange\n");
    printf("===================================================================\n");
    for (modeNum = 1; modeNum <= 10; modeNum++) {
        freqRange = GetFreqLimit(modeNum);
        bpm = GetBPM(modeNum);
        GetTempo(modeNum, &tempo);
        if (bpm != -1 && strcmp(tempo, "") != 0) {
            printf("\t%2d\t%14s\t\t%3d\t     %3d ~ %3d\n", modeNum, tempo, bpm, freqRange.min, freqRange.max);
        }
    }
    free(tempo);
}

void GetFrequency(setting* metronomeSetting) {
    bool exitLoop = metronomeSetting->modeNum != 0;
    bool exitInputLoop = false;
    char modeStr[1024], ch;
    do {
        printf("Choose tempo mode: ");
        strcpy(modeStr, "");
        fflush(stdout);
        while (!exitInputLoop) {
            ch = getchar();
            pthread_mutex_lock(&mutex);
            exitInputLoop = terminateProgram;
            pthread_mutex_unlock(&mutex);
            if (exitInputLoop) {}
            else if (ch == '\n') {
                exitInputLoop = true;
            }
            else {
                strncat(modeStr, &ch, 1);
            }
        }
        pthread_mutex_lock(&mutex);
        exitLoop = terminateProgram;
        pthread_mutex_unlock(&mutex);
        if (!exitLoop) {
            if (strcmp(modeStr, "") == 0) {
                printf(KYEL "[WARN] Input is empty.\n" KNRM);
                exitInputLoop = false;
            }
            else if (fabs(atoi(modeStr) - atof(modeStr)) > NEARLY_ZERO) {
                printf(KYEL "[WARN] Input is a floating point number.\n" KNRM);
                exitInputLoop = false;
            }
            else if (!IsMetronomeModeNumValid(atoi(modeStr))) {
                exitInputLoop = false;
            }
            else {
                metronomeSetting->modeNum = atoi(modeStr);
            }
        }
    } while (metronomeSetting->modeNum == 0 && !exitLoop);
    if (metronomeSetting->modeNum != 0) {
        metronomeSetting->frequency = GetBPM(metronomeSetting->modeNum);
    }
}

int ReadArrow() {
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
    int minBPM[] = {0, 1, 25, 46, 61, 67, 77, 109, 157, 177, 201};
    int maxBPM[] = {0, 24, 45, 60, 66, 76, 108, 156, 176, 200, 500};
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
                if (UpdateTimer(metronomeSetting.frequency) == -1) {
                    printf(KRED "\n[ERROR] Fail to set timer!\n" KNRM);
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
                if (UpdateTimer(metronomeSetting.frequency) == -1) {
                    printf(KRED "\n[Error] Fail to set timer!\n" KNRM);
                    TerminateProgram();
                }
            }
            else {
                PrintAdjustFreqInstructions(metronomeSetting.modeNum, metronomeSetting.frequency,
                                            "Minimum limit reached, unable to decrease further.");
            }
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
    sigset_t signalSet;
    char* tempo = "";
    bool terminateThread;
    printf("Start user input thread\n");
    sigemptyset(&signalSet);
    sigaddset(&signalSet, SIGINT);
    if (pthread_sigmask(SIG_UNBLOCK, &signalSet, NULL) == -1) {
        printf(KRED "[ERROR] Failed to set signal masks.\n" KNRM);
        TerminateProgram();
    }
    pthread_mutex_lock(&mutex);
    terminateThread = terminateProgram;
    pthread_mutex_unlock(&mutex);
    if (!terminateThread) {
        PrintTempo();
        if (metronomeSetting.modeNum == 0 || metronomeSetting.frequency == 0) {
            GetFrequency(&metronomeSetting);
        }
        pthread_mutex_lock(&mutex);
        terminateThread = terminateProgram;
        pthread_mutex_unlock(&mutex);
        if (!terminateThread) {
            if (UpdateTimer(metronomeSetting.frequency) == -1) {
                printf(KRED "\n[ERROR] Fail to set timer!\n" KNRM);
                TerminateProgram();
            }
            GetTempo(metronomeSetting.modeNum, &tempo);
            printf("Your tempo: " KGRN "%s" KNRM ", corresponding BPM: " KGRN "%d\n" KNRM,
                   tempo, metronomeSetting.frequency);
            metronomeSetting.frequency = AdjustFreq(metronomeSetting);
            ((setting*)defaultSetting)->frequency = metronomeSetting.frequency;
            free(tempo);
        }
    }
    printf("End user input thread\n");
    pthread_exit(NULL);
}

void* SpawnAudioThread(void* arg) {
    bool terminateThread;
    pthread_mutex_lock(&mutex);
    terminateThread = terminateProgram;
    pthread_mutex_unlock(&mutex);
    printf("Start audio thread\n");
    signal(SIGALRM, PrintAudio);
    if (timer_create(CLOCK_REALTIME, NULL, &timerid) == -1) {
        printf(KRED "[ERROR] Failed to create timer\n" KNRM);
        TerminateProgram();
    }
    while (!terminateThread) {
        pthread_mutex_lock(&mutex);
        terminateThread = terminateProgram;
        pthread_mutex_unlock(&mutex);
    }
    printf("End audio thread\n");
    timer_delete(timerid);
    pthread_exit(NULL);
}

void CtrlCHandler(int sigNum) {
    TerminateProgram();
}

void PrintAudio(int sigNum) {
    int i;
    static int count = 0;
    if (count >= 4) {
        printf("\rAudio: ");
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf("\b");
        }
        count = 0;
    }
    if (count % 2) {
        printf(KCYN "bob " KNRM);
        putchar(7);
        fflush(stdout);
    }
    else {
        printf(KMAG "beep " KNRM);
        putchar(7);
        fflush(stdout);
    }
    ++count;
}

int UpdateTimer(int frequency) {
    struct itimerspec timerInfo;
    timerInfo.it_value.tv_sec = 60 / frequency;
    timerInfo.it_value.tv_nsec = ((long long)60 * BILLION / frequency - (long long)timerInfo.it_value.tv_sec * BILLION);
    timerInfo.it_interval.tv_sec = 60 / frequency;
    timerInfo.it_interval.tv_nsec = ((long long)60 * BILLION / frequency - (long long)timerInfo.it_value.tv_sec * BILLION);
    return timer_settime(timerid, 0, &timerInfo, NULL);
}

void PrintAdjustFreqInstructions(int mode, int frequency, const char* warningMessage) {
    int i;
    char* tempo = "";
    printf("\033[3A\r");
    for (i = 0; i < TERMINAL_WIDTH; i++) {
        printf(" ");
    }
    GetTempo(mode, &tempo);
    printf("\rYour tempo: " KGRN "%s" KNRM ", corresponding BPM: " KGRN "%d\n" KNRM, tempo, frequency);
    printf("Please use arrow up and down key to adjust the frequency you want.\n");
    if (warningMessage != NULL) {
        printf(KYEL "[WARN] %s\n" KNRM, warningMessage);
        printf("Audio: ");
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf("\b");
        }
    }
    else {
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        printf("\n");
        for (i = 0; i < TERMINAL_WIDTH; i++) {
            printf(" ");
        }
        printf("\rAudio: ");
    }
    free(tempo);
}

void TerminateProgram() {
    pthread_mutex_lock(&mutex);
    terminateProgram = true;
    pthread_mutex_unlock(&mutex);
}

void ParseArgs(int argc, char** argv, char** filePath) {
    int i;
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (strcmp(*(argv + i), "--config") == 0) {
                if(argc == 2 || argc > 3) {
                    printf("usage: ./main [-h] [--config CONFIG_FILE_PATH] \n"
                           "main: error: argument %s: expected one argument\n", *(argv + i));
                    TerminateProgram();
                }
                else if (argc == 3) {
                    *filePath = malloc(strlen(*(argv + i + 1)) + 1);
                    strcpy(*filePath, *(argv + i + 1));
                }
                break;
            }
            else if ((strcmp(*(argv + i), "-h") == 0 || strcmp(*(argv + i), "--help") == 0)) {
                printf("usage: ./main [-h] [--config CONFIG_FILE_PATH] \n");
                if (argc > 2) {
                    printf("main: error: argument %s: expected no argument\n", *(argv + i));
                }
                else {
                    printf("\n"
                           "To start the real-time metronome.\n\n"
                           "optional arguments:\n"
                           " -h, --help                 show this help message and exit\n"
                           " --config CONFIG_FILE_PATH  to load the metronome setting from a configuration file\n");
                }
                TerminateProgram();
                break;
            }
            else {
                printf("usage: ./main [-h] [--config CONFIG_FILE_PATH]\n"
                       "main: error: unrecognized arguments: %s\n", *(argv + i));
                TerminateProgram();
                break;
            }
        }
    }
}

void ParseConfigFile(char* filePath, setting* metronomeSetting) {
    FILE* stream;
    char line[1024];
    char* parsedLine;
    bool isModeConfigFound = false, isFrequencyConfigFound = false;
    stream = fopen(filePath, "r");
    if (stream == NULL) {
        printf(KRED "[ERROR] File with path %s not found. \n" KNRM, filePath);
        TerminateProgram();
    }
    else {
        while ((fgets(line, 1024, stream)) != NULL) {
            if (strstr(line, "MODE")) {
                parsedLine = strstr(line, "=");
                metronomeSetting->modeNum = atoi(&parsedLine[1]);
                isModeConfigFound = true;
            }
            else if (strstr(line, "FREQUENCY")) {
                parsedLine = strstr(line, "=");
                metronomeSetting->frequency = atoi(&parsedLine[1]);
                isFrequencyConfigFound = true;
            }
        }
        fclose(stream);
        if (!isModeConfigFound || !isFrequencyConfigFound) {
            printf(KRED "[ERROR] Configuration file format error.\n" KNRM);
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
}

bool IsMetronomeModeNumValid(int modeNum) {
    if (modeNum <= 10 && modeNum >= 1) {
        return true;
    }
    else {
        printf(KRED "[ERROR] Setting with mode: %d is not within the valid range (1-10).\n" KNRM, modeNum);
        return false;
    }
}

bool IsMetronomeFrequencyValid(int modeNum, int frequency) {
    freqLimit frequencyRange;
    char* tempo = "";
    if (IsMetronomeModeNumValid(modeNum)) {
        frequencyRange = GetFreqLimit(modeNum);
        if (frequency <= frequencyRange.max && frequency >= frequencyRange.min) {
            return true;
        }
        else {
            GetTempo(modeNum, &tempo);
            printf(KRED "[ERROR] Setting with tempo: %s, with frequency: %d is not within the valid range (%d-%d).\n" KNRM,
                   tempo, frequency, frequencyRange.min, frequencyRange.max);
        }
    }
    free(tempo);
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

int GetBPM(int modeNum) {
    int BPM[] = {0, 12, 35, 53, 64, 72, 93, 133, 167, 189, 351};
    if(IsMetronomeModeNumValid(modeNum)) {
        return BPM[modeNum];
    }
    return -1;
}

void GetTempo(int modeNum, char** tempo) {
    char* tempoArr[] = {"", "Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", "Allegro", "Vivace",
                        "Presto", "Prestissimo"};
    if(IsMetronomeModeNumValid(modeNum)) {
        *tempo = malloc(strlen(tempoArr[modeNum]) + 1);
        strcpy(*tempo, tempoArr[modeNum]);
    }
    else {
        strcpy(*tempo, "");
    }
}
