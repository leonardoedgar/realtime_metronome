#include <stdio.h>
#include <stdlib.h>

// Type define
typedef struct
{
    int modenum;
    int frequency;
}setting;

typedef struct
{
    int min;
    int max;
}freqLimit;


//Prototype functions
int readArrow();
freqLimit getFreqLimit();
int adjustFreq(setting usersetting);


int main(void)
{
    setting userSetting;
        
    // userSetting = getfrequency();
    userSetting.frequency = 52;
    userSetting.modenum = 3;
    userSetting.frequency = adjustFreq(userSetting);
    
    printf("The current frequency is %d", userSetting.frequency);


    
    return 0;
}

int readArrow()
{
    int int_1;
    int int_2;
    int int_3;

    printf("Please using up and down key to adjust the frequency you want.\n");
    system ("/bin/stty raw");
    scanf("%d", &int_3);
    int_1 = getchar();
    int_2 = getchar();
    int_3 = getchar();
    system ("/bin/stty cooked");

    if (int_1 != 27)
    {
        printf("\nPlease enter an arrow key.\n");
    } 

    return int_3;
}




freqLimit getFreqLimit(int modenum)
{

    freqLimit frequencyRange;

    switch(modenum)
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

int adjustFreq(setting usersetting)
{
    int input; 

    freqLimit frequencyRange;
    frequencyRange = getFreqLimit(usersetting.modenum);
    input = readArrow();
    printf("\n");
    switch(input)
    {
        case 65:
            printf("Up!\n");
            if(frequencyRange.min < usersetting.frequency && usersetting.frequency < frequencyRange.max)
            {
                usersetting.frequency++;
            }
            else
            {
                printf("Limit reached\n");
            }
            
            break;
        case 66:
            printf("Down!\n");
            if(frequencyRange.min < usersetting.frequency && usersetting.frequency < frequencyRange.max)
            {
                usersetting.frequency--;
            }
            else
            {
                printf("Limit reached\n");
            }
            break;
        case 67:
            printf("Right!\n");
            break;
        case 68:
            printf("Left!\n");
            break;
    }
    return usersetting.frequency;

}