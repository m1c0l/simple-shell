#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "stream.h"
#include "filedesc.h"

void initStream(void) {
}

void endStream(void) {
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
    fprintf(stderr, "File not opened with read permissions: %d\n", in);
    return -1;
  }
  /* Error if outfile or errfile doesn't have write permission */
  if (!(outfile.writable || errfile.writable)) {
    fprintf(stderr, "File not opened with write permissions: %d\n", in);
    return -1;
  }

  if ((dup2(infile.fd, 0) == -1) ||
      (dup2(outfile.fd, 1) == -1) ||
      (dup2(errfile.fd, 2) == -1)) {
    fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}
