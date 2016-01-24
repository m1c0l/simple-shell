#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "stream.h"
#include "filedesc.h"

struct {
  int in;
  int out;
  int err;
  FILE *errFile;
} stdcopy;

void initStream(void) {
  stdcopy.in = dup(0);
  stdcopy.out = dup(1);
  stdcopy.err = dup(2);
  stdcopy.errFile = fdopen(stdcopy.err, "w");
  if (stdcopy.in == -1 || stdcopy.out == -1 || stdcopy.err == -1) {
    fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
  }
}

void endStream(void) {
  if ((close(stdcopy.in) == -1) ||
      (close(stdcopy.out) == -1) ||
      (close(stdcopy.err) == -1)) {
    fprintf(stderr, "Error closing file: %s\n", strerror(errno));
  }
  fclose(stdcopy.errFile);
}

int getStream(int fd) {
  if (fd == 0)
    return stdcopy.in;
  if (fd == 1)
    return stdcopy.out;
  return stdcopy.err; // fd == 2
}

FILE *getStderrFile(void) {
  return stdcopy.errFile;
}


int set_streams(int in, int out, int err) {
  file infile = getFile(in),
       outfile = getFile(out),
       errfile = getFile(err);

  /* if there was an error getting file info */
  if (infile.fd == -1 || outfile.fd == -1 || errfile.fd == -1)
    return -1;

  /* Error if infile doesn't have read permission */
  if (!infile.readable) {
    fprintf(getStderrFile(),
        "File not opened with read permissions: %d\n", in);
    return -1;
  }
  /* Error if outfile or errfile doesn't have write permission */
  if (!(outfile.writable || errfile.writable)) {
    fprintf(getStderrFile(),
        "File not opened with write permissions: %d\n", in);
    return -1;
  }

  if ((dup2(infile.fd, 0) == -1) ||
      (dup2(outfile.fd, 1) == -1) ||
      (dup2(errfile.fd, 2) == -1)) {
    fprintf(getStderrFile(),
        "Error duplicating file descriptor: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int reset_streams(void) {
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
