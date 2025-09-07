#include "ProcessHeader.h"

void ProcessHeader(unsigned char *headerBuffer, unsigned long *fileNameLen, unsigned long *fileLen)
{
    *fileNameLen = 0;
    *fileLen = 0;

    for (int i = 0; i < 8; i++) {
        *fileNameLen = (*fileNameLen << 8) | (unsigned char)headerBuffer[i];
    }
    for (int i = 0; i < 8; i++) {
        *fileLen = (*fileLen << 8) | (unsigned char)headerBuffer[8 + i];
    }
}