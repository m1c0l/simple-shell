#include "filedesc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int *g_fileDesc;
int g_currFileDesc = -1; // index of the highest file descriptor
size_t g_fileDescSize = 64; // dynamic size of file descriptor array

void initFileDesc() {
  g_fileDesc = (int *)malloc(sizeof(int) * g_fileDescSize);
  if (g_fileDesc == NULL) {
    fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
    exit(1);
  }
}

void endFileDesc() {
  free(g_fileDesc);
}

int openFile(char* filename, int oflag) {
  int fd = open(filename, oflag);
  if (fd == -1) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
    return 1;
  }

  if ((size_t)g_currFileDesc > g_fileDescSize) {
    g_fileDescSize *= 2;
    g_fileDesc = (int *)realloc(g_fileDesc, g_fileDescSize);
    if (g_fileDesc == NULL) {
      fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
      exit(1);
    }
  }

  g_currFileDesc++;
  g_fileDesc[g_currFileDesc] = fd;
  
  // printf("fd: %d\n", fd);
  return 0;
}
