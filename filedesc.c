#include "filedesc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

file *fileDesc;
int currFileDesc = -1; // index of the highest file descriptor
size_t fileDescSize = 64; // dynamic size of file descriptor array

struct {
  int in;
  int out;
  int err;
  FILE *errFile;
} stdcopy;

void initFileDesc() {
  fileDesc = (file*)malloc(sizeof(file) * fileDescSize);
  if (fileDesc == NULL) {
    fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
    exit(1);
  }

  stdcopy.in = dup(0);
  stdcopy.out = dup(1);
  stdcopy.err = dup(2);
  stdcopy.errFile = fdopen(stdcopy.err, "w");
  if (stdcopy.in == -1 || stdcopy.out == -1 || stdcopy.err == -1) {
    fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
  }
}

void endFileDesc() {
  free(fileDesc);

  if ((close(stdcopy.in) == -1) ||
      (close(stdcopy.out) == -1) ||
      (close(stdcopy.err) == -1)) {
    fprintf(stderr, "Error closing file: %s\n", strerror(errno));
  }
}

int openFile(char* filename, int oflag) {
  int fd = open(filename, oflag);
  if (fd == -1) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
    return 1;
  }

  if ((size_t)currFileDesc > fileDescSize) {
    fileDescSize *= 2;
    fileDesc = (file*)realloc(fileDesc, sizeof(file) * fileDescSize);
    if (fileDesc == NULL) {
      fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
      exit(1);
    }
  }

  currFileDesc++;
  fileDesc[currFileDesc].fd = fd;
  fileDesc[currFileDesc].oflag = oflag;
  
  return 0;
}

file getFile(int index) {
  if (index > currFileDesc) {
    fprintf(stdcopy.errFile, "Bad file descriptor: %d\n", index);
    file bad = { .fd = -1, .oflag = 0 };
    return bad;
  }
  return fileDesc[index];
}

int getStream(int fd) {
  if (fd == 0)
    return stdcopy.in;
  if (fd == 1)
    return stdcopy.out;
  else // fd == 2
    return stdcopy.err;
}

FILE *getStderrFile() {
  return stdcopy.errFile;
}

int reset_streams() {
  if ((dup2(stdcopy.in, 0) == -1) ||
      (dup2(stdcopy.out, 1) == -1) ||
      (dup2(stdcopy.err, 2) == -1)) {
    /* use the copied stderr in case the reset fails */
    fprintf(stdcopy.errFile,
        "Error duplicating file descriptor: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}
