#include "CreateFile.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>    
#include <string.h>    
#include <fcntl.h>      
#include <sys/stat.h>   
#include <sys/types.h>  
#include <unistd.h>     


int CreateFile(char *fileName,char *filePath,int *outputFile)
{
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
        *outputFile = open(newName, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (*outputFile != -1) {
            return 0;
        }

        // If we reach here, the file already exists or an error occurred
        if (errno != EEXIST) {
            perror("open");
            return -1; 
        }

        counter++;
        if (counter > 10000) {  // avoid infinite loop
            fprintf(stderr, "Too many file variants tried.\n");
            return -1;
        }
    }
} 