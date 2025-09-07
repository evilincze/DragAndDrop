#include "ClientLoop.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>  
#include <arpa/inet.h>    
#include <stdlib.h>
#include <sys/param.h>
#include <math.h>
#include <sys/stat.h>
#include "FilesStruct.h"

#define BUFFER_SIZE 1024

long getFileSize(const char *filename) 
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    else
        return -1;
}

void ClientLoop(int clientSocket, char *filePaths[],int fileCount)
{
    int inputFile;
    for(int i=0;i<fileCount;i++)
    {
        inputFile=open(filePaths[i],O_RDONLY);
        const char *fileName = strrchr(filePaths[i], '/');
        if (fileName)
            fileName++; // skip the '/'
        else
            fileName = filePaths[i];
        unsigned long fileNameSize=strlen(fileName);
        unsigned long fileSize = getFileSize(filePaths[i]);
        if(fileSize ==-1 || inputFile ==-1)
        {
            printf("File could not be read\n");
            return;
        }

        /* Header */
        unsigned char headerBuffer[16];
        for (int j = 0; j < 8; j++) {
            headerBuffer[7 - j] = (unsigned char)((fileNameSize >> (j * 8)) & 0xFF);
        }
        for (int j = 0; j < 8; j++) {
            headerBuffer[15 - j] = (unsigned char)((fileSize >> (j * 8)) & 0xFF);
        }
        write(clientSocket,headerBuffer,16);

        /*File Name*/
        write(clientSocket,fileName,strlen(fileName));

        /* Data */
        unsigned char buffer[BUFFER_SIZE];
        int totalBytesRead=0;
        while(true)
        {
            int bytesRead = read(inputFile,buffer + totalBytesRead,sizeof(buffer) - totalBytesRead);
            if(bytesRead == 0)// End of file
            {
                write(clientSocket,buffer,totalBytesRead);
                break;
            }
            totalBytesRead+=bytesRead;
            
            if (totalBytesRead >= BUFFER_SIZE - (BUFFER_SIZE / 10))// Send the data if buffer is almost full
            {
                write(clientSocket,buffer,totalBytesRead);
                totalBytesRead=0;
            }
        }

        printf("File %s succesfully send\n",fileName);
    }
}