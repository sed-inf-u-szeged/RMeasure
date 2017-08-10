#ifndef RMEASURE_H_INCLUDED
#define RMEASURE_H_INCLUDED

#ifdef STATIC_ANALYSIS
#define STATIC_BEGIN [[rpr::kernel]]{
#define STATIC_END }
#define DYNAMIC_BEGIN(NAME)
#define DYNAMIC_END

#endif // STATIC_ANALYSIS

#ifdef DYNAMIC_ANALYSIS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FIFO_FILE "../../RMeasureService/RMEASURE_FIFO"

inline void callFifo(const char* msg, int isBegin)
{
    int result = access (FIFO_FILE, F_OK);
    if(result == 0) {
        FILE *fp;
        fp = fopen(FIFO_FILE, "w");
        if (fp == NULL) {
            perror("fopen");
            exit(1);
        }
        if (isBegin == 1) {
            char rKernelName[strlen(msg)+5];
            memset(rKernelName, 0, sizeof rKernelName);
            strcat(rKernelName,"B:\0");
            strcat(rKernelName, msg);
            strcat(rKernelName, ";\0");
            fputs(rKernelName, fp);

        }
        else {
            fputs(msg, fp);
        }
        fclose(fp);
    }
}

#define STATIC_BEGIN
#define STATIC_END

#define DYNAMIC_BEGIN(NAME) callFifo(NAME, 1);

#define DYNAMIC_END callFifo("E;", 0);

#endif // DYNAMIC_ANALYSIS

#endif // RMEASURE_H_INCLUDED


