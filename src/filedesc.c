#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "filedesc.h"
#include "stream.h"
#include "util.h"

file *fileDesc;
int currFileDesc = -1; // index of the highest file descriptor
size_t fileDescSize = 64; // dynamic size of file descriptor array

void initFileDesc(void) {
  fileDesc = (file*)malloc(sizeof(file) * fileDescSize);
  if (fileDesc == NULL) {
    fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
    exit(1);
  }
}

void endFileDesc(void) {
  free(fileDesc);
}

int openFile(char* filename, int oflag) {
  int fd = open(filename, oflag, 0644);
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
  if (index < 0 || index > currFileDesc || fileDesc[index].fd < 0) {
    fprintf(getStderrFile(), "Bad file descriptor: %d\n", index);
    file bad = { .fd = -1, .oflag = 0 };
    return bad;
  }
  return fileDesc[index];
}

int closeFile(char* fd_string) {
  int fd = get_file_desc(fd_string);
  int real_fd = getFile(fd).fd;
  if (real_fd < 0) return -1; // invalid fd

  close(real_fd);

  /* mark this file descriptor as invalid */
  fileDesc[fd].fd = -1;

  return 0;
}
