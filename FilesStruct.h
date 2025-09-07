#ifndef FILESTRUCT_H
#define FILESTRUCT_H

typedef struct {
    char **files;
    int size;     // number of elements currently stored
    int capacity; // total allocated slots
} FilesStruct;

// Initialize
void initFiles(FilesStruct *v); 

// Append an element
void pushBack(FilesStruct *v, char *file);

// Free memory
void freeFiles(FilesStruct *v);

#endif