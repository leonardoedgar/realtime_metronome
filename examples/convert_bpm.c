#include <stdio.h>
#include <time.h>

#define BILLION		1000000000L

typedef struct {
    time_t tv_sec;
    long tv_nsec;
} TimeSpec;

int main() {
    int frequency = 53;
    TimeSpec time;
    time.tv_sec = 60/frequency;
    time.tv_nsec = (60*BILLION/frequency - time.tv_sec*BILLION);
    printf("Timer sec : %d\n", (int)time.tv_sec);
    printf("Timer nsec: %ld\n", time.tv_nsec);
    return 0;
}
