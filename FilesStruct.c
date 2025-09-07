#include "FilesStruct.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>


// Initialize
void initFiles(FilesStruct *v) {
    v->size = 0;
    v->capacity = 20; // Initial capacity
    v->files = (char **)malloc(sizeof(char*) * v->capacity);
}

// Append an element
void pushBack(FilesStruct *v, char *file) {
    if (v->size == v->capacity) {
        v->capacity *= 2;
        char **tmp = realloc(v->files, sizeof(char*) * v->capacity);
        if (!tmp) {
            perror("realloc");
            exit(1);
        }
        v->files = tmp;
    }
    v->files[v->size++] = file;
}

// Free memory
void freeFiles(FilesStruct *v) {
    for (int i = 0; i < v->size; i++) {
        free(v->files[i]);  
    }
    free(v->files);        
    v->files = NULL;
    v->size = 0;
    v->capacity = 0;
}