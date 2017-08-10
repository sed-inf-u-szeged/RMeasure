/*
* compile: gcc example02.c -o example02 -DDYNAMIC_ANALYSIS
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "rmeasure.h"

int main(void)
{
    DYNAMIC_BEGIN("example02_kernel1")
    printf("I'll be back in 10 seconds...\n\n");
    sleep(10);
    printf("I'm back!");
    DYNAMIC_END
    sleep(1);
    DYNAMIC_BEGIN("example02_kernel1")
    printf("I'll be back in 2 seconds...\n\n");
    sleep(2);
    printf("I'm back!");
    DYNAMIC_END

    sleep(1);

    DYNAMIC_BEGIN("example02_kernel2")
    sleep(5);
    DYNAMIC_END


    return(0);
}