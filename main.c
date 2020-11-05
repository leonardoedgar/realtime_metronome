#include <stdio.h>
#include <stdlib.h>

// Type define
typedef struct
{
    char modetemp[1024];
    int modenum;
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

char *tempo[] = {"", "Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", "Allegro", "Vivace", "Presto", "Prestissimo"};
int BPM[] = {0, 12, 35, 53, 64, 72, 93, 133, 167, 189, 351};
int minBPM[] = {0, 0, 25, 46, 61, 67, 77, 109, 157, 177, 201};
int maxBPM[] = {0, 24, 45, 60, 66, 76, 108, 156, 176, 200, 500};

void printtempo()
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

int main(void)
{
    setting userSetting;

    userSetting = getfrequency(userSetting);
    userSetting.frequency = adjustFreq(userSetting);

    return 0;
}

setting getfrequency(setting userSetting)
{
    // Print out the list to choose modenum and basical frequency
    printtempo();
    do
    {
        printf("Please choose tempo mode:");
        if (!fgets(userSetting.modetemp, 1024, stdin))
        {
            continue;
        }
        userSetting.modenum = atoi(userSetting.modetemp);
        if (userSetting.modenum > 9)
            userSetting.modenum = 0;
    } while (userSetting.modenum == 0);
    printf("Your mode: %d, Corresponding BPM: %d\n", userSetting.modenum, userSetting.frequency = BPM[userSetting.modenum]);
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

freqLimit getFreqLimit(int modenum)
{

    freqLimit frequencyRange;

    switch (modenum)
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
    int status = 1;

    freqLimit frequencyRange;
    frequencyRange = getFreqLimit(userSetting.modenum);
    while (status)
    {
        input = readArrow();
        switch (input)
        {
        case 65:
            printf("\n");
            if (userSetting.frequency < frequencyRange.max)
            {
                userSetting.frequency++;
            }
            else
            {
                printf("[WARN]  Maximum limit reached, can't add more.\n");
            }
            break;
        case 66:
            printf("\n");
            if (frequencyRange.min < userSetting.frequency)
            {
                userSetting.frequency--;
            }
            else
            {   
                printf("[WARN]  Minimum limit reached, can't reduce more.\n");
            }
            break;
        case 67:
            printf("Right!\n");
            break;
        case 68:
            printf("Left!\n");
            break;
        case 'q':
            status = 0;

            break;
        default:
            status = 0;
            printf("end editing\n");
            break;
        }
        printf("The current frequency is %d.\n", userSetting.frequency);
        fflush(stdout);
    }

    return userSetting.frequency;
}