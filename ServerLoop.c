#include "ServerLoop.h"
#include "CreateFile.h"
#include "ProcessHeader.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>   // for sockaddr_in, htons, INADDR_ANY
#include <arpa/inet.h>    // for htons(), inet_addr(), etc.
#include <stdlib.h>
#include <sys/param.h>
#include <math.h>

#define BUFFER_SIZE 1024
char *filePath="/home/evilin/Downloads";

void ServerLoop(int clientSocket)
{
    unsigned char buffer[BUFFER_SIZE];
    unsigned char bufferForData[BUFFER_SIZE];
    unsigned char headerBuffer[16];
    char *fileName;
    unsigned long fileNameLen;
    unsigned long fileLen;

    unsigned long totalBytesRead=0;
    unsigned int bufferForDataId=0;

    int outputFile;

    while(true)
    {
        int bytesRead=recv(clientSocket,buffer,sizeof(buffer),0);
        if(bytesRead == 0)
            break;
        for(int i=0;i<bytesRead;i++)
        {
            if(totalBytesRead < 16)//reading header
            {
                headerBuffer[totalBytesRead] = buffer[i];
            }
            if(totalBytesRead == 15)//process header
            {
                ProcessHeader(headerBuffer,&fileNameLen,&fileLen);
                fileName = (char*) malloc(sizeof(char)*fileNameLen +1);
            }
            if(totalBytesRead > 15 && totalBytesRead <= (fileNameLen + 15))//reading filename
            {
                fileName[totalBytesRead-16] = buffer[i];
            }
            if(totalBytesRead == (fileNameLen + 15))//open file
            {
                fileName[fileNameLen]='\0';
                printf("Reading file %s\n",fileName);
                CreateFile(fileName,filePath,&outputFile);
            }
            if(totalBytesRead > (fileNameLen + 15) && totalBytesRead <= (15+fileNameLen+fileLen))//write into file
            {
                /* buffering the data to be written */
                bufferForData[bufferForDataId]=buffer[i];
                bufferForDataId++;
                if(bufferForDataId == BUFFER_SIZE)// write the data down
                {
                    write(outputFile,bufferForData,bufferForDataId);
                    bufferForDataId=0;
                }
            }

            if(totalBytesRead == (15+fileNameLen+fileLen))//file transmited
            {
                /* write down the remaining data */
                write(outputFile,bufferForData,bufferForDataId);
                bufferForDataId=0;

                printf("Transmission completed\n");
                free(fileName);
                close(outputFile);
                totalBytesRead =0;
                continue;
            }
            totalBytesRead+=(long)1;
        }
    }
}