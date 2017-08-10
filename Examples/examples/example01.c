/*
* compile: gcc example01.c -o example01 -DDYNAMIC_ANALYSIS
*/
#include <stdlib.h>
#include "rmeasure.h"

int main(void)
{

    int i = 0;
    int n = 10000 * 20000;

    int *block = malloc(n * sizeof(int));
    if (block != NULL) {
        DYNAMIC_BEGIN("example01_kernel1")
        for (i = 0; i < n/2; i++) {
             int ri = rand() % n;
             block[ri] = 0;
        }
        DYNAMIC_END
        free(block);
    }
    
    return 0;
}
