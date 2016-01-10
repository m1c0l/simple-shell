#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "command.h"
#include "filedesc.h"

int execute_command(command_data cmd_data);
command_data parse_command(int argc, char **argv, int *opt);

int stdinCopy, stdoutCopy, stderrCopy;

void set_streams(int in, int out, int err) {
  stdinCopy = dup(0);
  stdoutCopy = dup(1);
  stderrCopy = dup(2);
  printf("stdin %d stdout %d\n", stdinCopy, stdoutCopy);
  dup2(g_fileDesc[in], STDIN_FILENO);
  dup2(g_fileDesc[out], STDOUT_FILENO);
  dup2(g_fileDesc[err], STDERR_FILENO);
}

int command(int argc, char **argv, int *opt) {
  command_data data = parse_command(argc, argv, opt);
  set_streams(data.in, data.out, data.err);
  return execute_command(data);
}

command_data parse_command(int argc, char **argv, int *opt) {
  /* Find index of the next argument starting with "--" */
  int arg_count = 0;
  int opt_ind = *opt;
  for (int i = opt_ind; i < argc &&
      !(argv[i][0] == '-' && argv[i][1] == '-'); i++) {
    arg_count++;
  }

  /* Need at least 4 arguments */
  if (arg_count < 4) {
    fprintf(stderr, "Not enough arguments for --command\n");
  }

  command_data cmd_data;
  /* first 3 arguments are input, output, and error */
  cmd_data.in = atoi(argv[opt_ind++]);
  cmd_data.out = atoi(argv[opt_ind++]);
  cmd_data.err = atoi(argv[opt_ind++]);

  /* size of command's argv */
  int argv_size = arg_count - 3;

  /* allocate space for argv, with a null terminator */
  cmd_data.argv = (char**)malloc(sizeof(char*) * (argv_size+1));
  cmd_data.argv[argv_size] = NULL;

  /* assign arguments to the command's argv */
  for (int i = 0; i < argv_size; i++) {
    cmd_data.argv[i] = argv[opt_ind];
    opt_ind++;
  }

  /* print the command's data */
  printf("in: %d\n", cmd_data.in);
  printf("out: %d\n", cmd_data.out);
  printf("err: %d\n", cmd_data.err);
  for (int i = 0; cmd_data.argv[i] != NULL; i++)
    printf("argv[%d]: %s\n", i, cmd_data.argv[i]);

  /* set new optind and returns the parsed data */
  *opt = opt_ind;
  return cmd_data;
}

int execute_command(command_data cmd_data) {
  int pid;
  int status;
  pid = fork();
  if (pid == 0) {
    execvp(cmd_data.argv[0], cmd_data.argv);
    return 1;
  }
  else {
    waitpid(pid, &status, 0);
    dup2(stdinCopy, 0);
    dup2(stdoutCopy, 1);
    close(stdinCopy);
    close(stdoutCopy);
    close(stderrCopy);
    return WEXITSTATUS(status);
  }

  return 0;
}
