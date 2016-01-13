#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include "command.h"
#include "filedesc.h"
#include "util.h"

int execute_command(command_data cmd_data);
command_data parse_command(int argc, char **argv, int *opt);
int set_streams(int in, int out, int err);

int command(int argc, char **argv, int *opt) {
  command_data data = parse_command(argc, argv, opt);
  /* error parsing command */
  if (data.argv == NULL)
    return 1;

  if (set_streams(data.in, data.out, data.err) == -1) {
    reset_streams();
    return 1;
  }

  int ret = execute_command(data);
  /* if the command fails, write an error to the command's stderr */
  if (ret)
    fprintf(stderr, "%s: %s\n", data.argv[0], strerror(errno));

  if (reset_streams() == -1) {
    return 1;
  }
  free(data.argv); // malloc called in parse_command()
  return ret;
}

int set_streams(int in, int out, int err) {
  file infile = getFile(in),
       outfile = getFile(out),
       errfile = getFile(err);

  /* if there was an error getting file info */
  if (infile.fd == -1 || outfile.fd == -1 || errfile.fd == -1)
    return -1;

  /* Error if infile is write-only */
  if (infile.oflag & O_WRONLY) {
    fprintf(getStderrFile(),
        "File not opened with read permissions: %d\n", in);
    return -1;
  }
  /* Error if outfile or errfile doesn't have write permission */
  if (!(outfile.oflag & (O_WRONLY|O_RDWR)) ||
      !(errfile.oflag & (O_WRONLY|O_RDWR))) {
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

command_data parse_command(int argc, char **argv, int *opt) {
  /* Find index of the next argument starting with "--" */
  int arg_count = 0;
  int opt_ind = *opt;
  for (int i = opt_ind; i < argc && is_not_option(argv[i]); i++) {
    arg_count++;
  }

  /* struct to store data */
  command_data cmd_data;

  /* Need at least 4 arguments */
  if (arg_count < 4) {
    fprintf(stderr, "Not enough arguments for --command\n");
    cmd_data.argv = NULL;
    return cmd_data;
  }

  /* first 3 arguments are input, output, and error */
  char *endptr;
  char *str = argv[opt_ind++];
  long val;
  val = strtol(str, &endptr, 10);
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
          || (errno != 0 && val == 0)) { // out of range
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (endptr == str) { // no digits found
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (*endptr != '\0') { // letters found after #
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (val < 0) { // negative fd
    fprintf(stderr, "Bad file descriptor\n");
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.in = val;


  str = argv[opt_ind++];
  val = strtol(str, &endptr, 10);
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
          || (errno != 0 && val == 0)) {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (endptr == str) {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (*endptr != '\0') {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (val < 0) {
    fprintf(stderr, "Bad file descriptor\n");
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.out = val;


  str = argv[opt_ind++];
  val = strtol(str, &endptr, 10);
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
          || (errno != 0 && val == 0)) {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (endptr == str) {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (*endptr != '\0') {
      fprintf(stderr, "Bad file descriptor\n");
      cmd_data.argv = NULL;
      return cmd_data;
  }
  if (val < 0) {
    fprintf(stderr, "Bad file descriptor\n");
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.err = val;

  /* size of command's argv */
  int argv_size = arg_count - 3;

  /* allocate space for argv, with a null terminator */
  cmd_data.argv = (char**)malloc(sizeof(char*) * (argv_size+1));
  if (cmd_data.argv == NULL) {
    fprintf(stderr, "Memory error: %s\n", strerror(errno));
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.argv[argv_size] = NULL;

  /* assign arguments to the command's argv */
  for (int i = 0; i < argv_size; i++) {
    cmd_data.argv[i] = argv[opt_ind];
    opt_ind++;
  }

  /* set new optind and returns the parsed data */
  *opt = opt_ind;
  return cmd_data;
}

int execute_command(command_data cmd_data) {
  int pid;
  //int status;
  pid = fork();
  if (pid == -1) {
    fprintf(stderr, "Error forking child: %s\n", strerror(errno));
    return -1;
  }

  if (pid == 0) {
    execvp(cmd_data.argv[0], cmd_data.argv);
    /* if the child process reaches here, there was an error */
    fprintf(stderr, "Error executing: %s\n", strerror(errno));
    return 1;
  }
  else {
    // Code for --wait; don't need for part 1a
    /*
    if (waitpid(pid, &status, 0) == -1) {
      fprintf(stderr, "Error waiting for child: %s\n", strerror(errno));
      return 1;
    }
    return WEXITSTATUS(status);
    */
    return 0;
  }
}
