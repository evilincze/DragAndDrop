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
#include <sys/stat.h>
#include <dirent.h>


#include "ServerLoop.h"
#include "ClientLoop.h"
#include "FilesStruct.h"


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
        ServerLoop(clientSocket);
        printf("Connection ended\n");
    }
}

int FileOrFolder(char *filePath)
{
    /*  Returns 0 if file
                1 if folder
                2 if anything else (ignore) */
    
    struct stat st;
    if (stat(filePath, &st) != 0) {
        perror("stat");
        return 1;
    }

    if (S_ISDIR(st.st_mode)) {
        return 1;
    } else if (S_ISREG(st.st_mode)) {
        return 0;
    } else {
        return 2;
    }
}

int FindAllFiles(char *folderPath,FilesStruct *files)
{
    DIR *dir = opendir(folderPath);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);

        struct stat st;
        if (stat(fullPath, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISREG(st.st_mode))// Is file 
        {
            size_t len = strlen(folderPath) + 1 + strlen(entry->d_name) + 1; 
            char *fullPath = malloc(len);
            if (!fullPath) {
                perror("malloc failed");
                exit(1);
            }

            snprintf(fullPath, len, "%s/%s", folderPath, entry->d_name);

            pushBack(files,fullPath);
        } 

    }
} 

void runAsClient(int argn,char *args[])
{
    char *ip =args[2];

    int mysock =socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in myadd;
    myadd.sin_family=AF_INET;
    myadd.sin_port=htons(11000);

    inet_pton(AF_INET,ip,&myadd.sin_addr);

    if(mysock == -1)
    {
        printf("Error establishing connection (socket)\n");
        return;
    }
    if(connect(mysock,(struct sockaddr*)&myadd,sizeof(myadd))== -1)
    {
        printf("Error establishing connection (connect)\n");
        return;
    }

    FilesStruct files;
    initFiles(&files);
    for(int i=3;i<argn;i++)
    {
        printf("Trying filePath:%s\n",args[i]);
        if(FileOrFolder(args[i]) == 0)
        {
            char* f = (char*)malloc(sizeof(char)*strlen(args[i]));
            if (!f) 
            {
                perror("malloc failed");
                exit(1);
            }
            strcpy(f,args[i]);
            pushBack(&files,f);
        }
        if(FileOrFolder(args[i]) == 1)
            FindAllFiles(args[i],&files);
    }

    ClientLoop(mysock,files.files,files.size);
    freeFiles(&files);
    printf("All files send\n");
    close(mysock);
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
            {
                printf("Drag and Drop active as a client...\n");
                runAsClient(argn,args);
            }
            else
            {
                printf("Not enough argumets for running as client.\n");
                return 1;
            }
        }
    }
}