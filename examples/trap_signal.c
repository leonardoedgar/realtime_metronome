#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

static char done = 0;
static void sigHandler(int signum) {
    done = 1;
}

int main(void) {
    struct sigaction sa;
    char ch;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigHandler;
    sa.sa_flags = 0;// not SA_RESTART!;

    sigaction(SIGINT, &sa, NULL);

    while (ch != 'q' && !done) {
        printf("done:  %d\n", done);
        ch = getchar();
        printf("char: %d\n", (int)ch==-1);
        usleep(1000);
    }
    printf("%d ", done);
    printf("exiting\n");
    return 0;
}
