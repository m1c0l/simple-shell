#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "stream.h"
#include "filedesc.h"

int set_streams(int in, int out, int err) {
  file infile = getFile(in), outfile = getFile(out), errfile = getFile(err);

  if ((dup2(infile.fd, 0) == -1) ||
      (dup2(outfile.fd, 1) == -1) ||
      (dup2(errfile.fd, 2) == -1)) {
    fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
    return -1;
  }

  close_all_files();
  return 0;
}

int invalid_files(file inf, file outf, file errf, command_data cmd_data) {

  if (inf.fd == -1) {
    fprintf(stderr, "Error using file descriptor: %d\n", cmd_data.in);
    return 1;
  }
  if (outf.fd == -1) {
    fprintf(stderr, "Error using file descriptor: %d\n", cmd_data.out);
    return 1;
  }
  if (errf.fd == -1) {
    fprintf(stderr, "Error using file descriptor: %d\n", cmd_data.err);
    return 1;
  }

  /* Error if infile doesn't have read permission */
  if (!inf.readable) {
    fprintf(stderr, "File opened without read permission: %d\n", cmd_data.in);
    return 1;
  }
  /* Error if outfile or errfile doesn't have write permission */
  if (!outf.writable || !errf.writable) {
    fprintf(stderr, "File opened without write permission: %d\n", cmd_data.out);
    return 1;
  }
  return 0;
}
