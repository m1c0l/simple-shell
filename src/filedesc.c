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

void close_all_files(void) {
  for (int i = 0; i <= currFileDesc; i++) {
    close(fileDesc[i].fd);
  }
}

void endFileDesc(void) {
  close_all_files();
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
  // Store file info
  file* curr = &fileDesc[currFileDesc];
  curr->fd = fd;
  if (oflag & O_WRONLY) {
    curr->readable = 0;
    curr->writable = 1;
  }
  else if (oflag & O_RDWR) {
    curr->readable = 1;
    curr->writable = 1;
  }
  else {
    curr->readable = 1;
    curr->writable = 0;
  }
  
  return 0;
}

file getFile(int index) {
  if (index < 0 || index > currFileDesc || fileDesc[index].fd < 0) {
    fprintf(stderr, "Bad file descriptor: %d\n", index);
    file bad = { .fd = -1 };
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


int openPipe() {
  if ((size_t)(currFileDesc + 1) > fileDescSize) {
    fileDescSize *= 2;
    fileDesc = (file*)realloc(fileDesc, sizeof(file) * fileDescSize);
    if (fileDesc == NULL) {
      fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
      exit(1);
    }
  }

  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    return -1;
  }

  currFileDesc++;
  // Store fd info for read end of the pipe
  file* curr = &fileDesc[currFileDesc];
  curr->fd = pipe_fd[0];
  curr->readable = 1;
  curr->writable = 0;

  currFileDesc++;
  // Store fd info for write end of the pipe
  curr = &fileDesc[currFileDesc];
  curr->fd = pipe_fd[1];
  curr->readable = 0;
  curr->writable = 1;

  return 0;
}
