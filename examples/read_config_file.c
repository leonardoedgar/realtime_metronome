#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int get_config(char* filename, char* config) {
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool keyFound;
    int configData = 0;
    stream = fopen(filename, "r");
    if (stream == NULL) {
        printf("FileNotFound\n");
    }
    else {
        while ((read = getline(&line, &len, stream)) != -1) {
            if (strstr(line, config)) {
                line = strstr(line, "=");
                keyFound = true;
                break;
            }
        }
        if (keyFound) {
            configData = atoi(&line[1]);
        }
        fclose(stream);
    }
    return configData;
}

int main() {
    int bpm = get_config("config.txt", "BPM");
    printf("BPM = %d\n", bpm);
    int apple = get_config("config.txt", "APPLE");
    printf("APPLE = %d\n", apple);
    return 0;
}