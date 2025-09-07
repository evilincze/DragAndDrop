#include "CreateFile.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>     // for exit codes (if needed)
#include <string.h>     // for string functions (if needed)
#include <fcntl.h>      // for open() flags like O_CREAT, O_WRONLY, O_EXCL
#include <sys/stat.h>   // for file permission constants (0644, etc.)
#include <sys/types.h>  // for mode_t and ssize_t
#include <unistd.h>     // for close(), write(), etc.


int CreateFile(char *fileName,char *filePath,int *outputFile)
{
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
        *outputFile = open(newName, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (*outputFile != -1) {
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