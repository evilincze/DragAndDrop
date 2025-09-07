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

/*static uint64_t ntohll(uint64_t x) {
    uint32_t hi = ntohl((uint32_t)(x >> 32));
    uint32_t lo = ntohl((uint32_t)(x & 0xFFFFFFFFULL));
    return ((uint64_t)lo << 32) | hi;
}*/
/*void printByteArray(char *arr,int len)
{
    for(int i=0;i<len;i++)
    {
        printf("%d ",arr[i]);
    }
}*/

/*void cpWithOffset(char *a,int offset, char *b,int len)
{
    // a + offset <- b till len
    for(int i=0;i<len;i++)
    {
        a[offset+i] =b[i];
    }
}*/

int outputFile;
int CreateFile(char *fileName) {
    //printf("Attenmting creating a file!");
    //outputFile = open("test.txt", O_WRONLY | O_CREAT | O_EXCL, 0644);
    //return 0;

    char newName[256];
    int counter = 0;

    // Try original name + variants
    while (1) {
        if (counter == 0) {
            snprintf(newName, sizeof(newName), "%s/%s",filePath, fileName);
        } else {
            snprintf(newName, sizeof(newName), "%s/%s%d", filePath,fileName, counter);
        }

        // Try to create the file (fail if it already exists)
        outputFile = open(newName, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (outputFile != -1) {
            //printf("Created file: %s\n", newName);
            return 0;
        }

        // If we reach here, the file already exists or an error occurred
        if (errno != EEXIST) {
            perror("open");
            return -1; // some error other than "already exists"
        }

        counter++;
        if (counter > 10000) {  // avoid infinite loop
            fprintf(stderr, "Too many file variants tried.\n");
            return -1;
        }
    }
}

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

bool DoServerAction1(int clientSocket)
{
    unsigned char buffer[BUFFER_SIZE];
    unsigned char bufferForData[BUFFER_SIZE];
    unsigned char headerBuffer[16];
    char *fileName;
    unsigned long fileNameLen;
    unsigned long fileLen;

    unsigned long totalBytesRead=0;
    unsigned int bufferForDataId=0;
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
                CreateFile(fileName);
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

void runAsServer()
{
    printf("Drag and Drop active as a server...\n");
    int mysock =socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in myadd;
    myadd.sin_family=AF_INET;
    myadd.sin_port=htons(11000);
    myadd.sin_addr.s_addr=INADDR_ANY;

    if(mysock == -1)
    {
        printf("Error establishing connection (socket)\n");
        return;
    }


    if(bind(mysock,(struct sockaddr*)&myadd,sizeof(myadd)) == -1)
    {
        printf("Error establishing connection (bind)\n");
        return;
    }
    if(listen(mysock,5) == -1)
    {
        printf("Error establishing connection (bind)\n");
        return;
    }

    while(true)
    {
        int clientSocket=accept(mysock,NULL,NULL);
        printf("Connection established\n");
        while(true)
            DoServerAction1(clientSocket);
        printf("Connection ended\n");
    }
}

void runAsClient(char *ip)
{

}

int main(int argn, char* args[])
{
    if(argn > 1)
    {
        if (strcmp(args[1], "-s") == 0)
            runAsServer();
        if(strcmp(args[1], "-c") == 0)
        {
            if(argn >2)
                runAsClient(args[2]);
            else
            {
                printf("Not enough argumets for running as client.\n");
                return 1;
            }
        }
    }
}

