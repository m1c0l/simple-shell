#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>

#include "command.h"
#include "filedesc.h"
#include "stream.h"
#include "util.h"

typedef struct {
  char **argv;
  pid_t pid;
} wait_t;

wait_t *wait_data;
int wait_data_index = 0;
size_t wait_data_size = 64;

int execute_command(command_data cmd_data);

int command(command_data cmd_data) {
  /* error parsing command */
  if (cmd_data.argv == NULL)
    return 1;

  file inf = getFile(cmd_data.in),
       outf = getFile(cmd_data.out),
       errf = getFile(cmd_data.err);

  if (invalid_files(inf, outf, errf, cmd_data)) {
    free(cmd_data.argv);
    return 1;
  }

  int ret = execute_command(cmd_data);

  return ret;
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

  // cmd_data.argc = arg_count - 3;
  /* Need at least 4 arguments */
  if (arg_count < 4) {
    fprintf(stderr, "Not enough arguments for --command\n");
    cmd_data.argv = NULL;
    return cmd_data;
  }

  /* first 3 arguments are input, output, and error */
  int fd_number = get_file_desc(argv[opt_ind++]);
  if (fd_number == -1) {
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.in = fd_number;

  fd_number = get_file_desc(argv[opt_ind++]);
  if (fd_number == -1) {
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.out = fd_number;

  fd_number = get_file_desc(argv[opt_ind++]);
  if (fd_number == -1) {
    cmd_data.argv = NULL;
    return cmd_data;
  }
  cmd_data.err = fd_number;

  /* size of command's argv */
  int argv_size = arg_count - 3;

  /* allocate space for argv, with a null terminator */
  cmd_data.argv = (char**)malloc(sizeof(char*) * (argv_size+1));
  if (cmd_data.argv == NULL) {
    fprintf(stderr, "Memory error: %s\n", strerror(errno));
    return cmd_data;
  }
  cmd_data.argv[argv_size] = NULL;

  /* assign arguments to the command's argv */
  for (int i = 0; i < argv_size; i++) {
    cmd_data.argv[i] = argv[opt_ind];
    opt_ind++;
  }

  /* set new optind */
  *opt = opt_ind;

  /* return the parsed data */
  return cmd_data;
}

int execute_command(command_data cmd_data) {

  int pid;
  pid = fork();
  if (pid == -1) {
    fprintf(stderr, "Error forking child: %s\n", strerror(errno));
    free(cmd_data.argv);
    return 1;
  }

  if (pid == 0) {
    /* Child process */

    if (set_streams(cmd_data.in, cmd_data.out, cmd_data.err) == -1) {
      exit(1);
    }

    execvp(cmd_data.argv[0], cmd_data.argv);
    /* if the child process reaches here, there was an error */
    fprintf(stderr, "Error executing: %s\n", strerror(errno));
    return 1;
  }
  else {
    /* Parent process */

    /* store child process info */
    if ((size_t)wait_data_index >= wait_data_size) {
      wait_data_size *= 2;
      wait_data = (wait_t*)realloc(wait_data, sizeof(wait_t) * wait_data_size);
      if (wait_data == NULL) {
        fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
        exit(1);
      }
    }
    wait_data[wait_data_index].pid = pid;
    wait_data[wait_data_index].argv = cmd_data.argv;
    wait_data_index++;
    return 0;
  }
}

void initCommand(void) {
  wait_data = (wait_t*)malloc(sizeof(wait_t) * wait_data_size);
  if (wait_data == NULL) {
    fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
    exit(1);
  }
}

void free_wait_data(void) {
  /* Free dynamic memory */
  for (int i = 0; i < wait_data_index; i++) {
    free(wait_data[i].argv);
  }
  free(wait_data);
}

int endCommand(int wait_flag, int profile_flag) {
  if (!wait_flag) {
    free_wait_data();
    return 0;
  }

  pid_t pid;
  int status = 0;
  int max_status = 0;

  struct rusage curr_usage, prev_usage;

  /* sets prev_usage time to zero */
  if (profile_flag) {
    if (getrusage(RUSAGE_CHILDREN, &prev_usage))
      perror("getrusage");
  }

  while ((pid = waitpid(-1, &status, 0))) {
    if (errno == ECHILD) // all children reaped
      break;

    if (pid == -1) {
      printf("Error waiting for child: %s\n", strerror(errno));
      return 1;
    }

    int exit_status = WEXITSTATUS(status);
    if (exit_status > max_status)
      max_status = exit_status;

    /* find the wait_data with the current pid */
    int index;
    for (index = 0; index < wait_data_index; index++)
      if (wait_data[index].pid == pid)
        break;
    if (index >= wait_data_index) { // make sure we found the pid
      fprintf(stderr, "Process ID not found: %d\n", pid);
      return 1;
    }

    /* Print exit status and command's argv */
    wait_t *curr = &wait_data[index];
    printf("%d", exit_status);
    for (int i = 0; curr->argv[i] != NULL; i++) {
      printf(" %s", curr->argv[i]);
    }
    printf("\n");

    if (profile_flag) {
      if (getrusage(RUSAGE_CHILDREN, &curr_usage))
        perror("getrusage");
      printf("User: %fs\tSystem: %fs\n",
          get_time_diff(prev_usage.ru_utime, curr_usage.ru_utime),
          get_time_diff(prev_usage.ru_stime, curr_usage.ru_stime));

      prev_usage = curr_usage;
    }
  }

  free_wait_data();

  return max_status;
}
