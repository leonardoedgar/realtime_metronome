#include <stdio.h>
#include <stdlib.h>


int read_arrow();

int main(void)
{
    int input;    
    
    input = read_arrow();
    printf("\n");
    switch(input)
    {
        case 65:
            printf("Up!\n");
            break;
        case 66:
            printf("Down!\n");
            break;
        case 67:
            printf("Right!\n");
            break;
        case 68:
            printf("Left!\n");
            break;
    }
    return 0;
}

int read_arrow()
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

    switch(frequency)
    {
        case 1
    }