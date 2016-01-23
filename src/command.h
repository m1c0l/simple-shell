#ifndef COMMAND_H
#define COMMAND_H

typedef struct {
  int in;
  int out;
  int err;
  // int argc;
  char** argv;
} command_data;

/* Sets optind while consuming --command args */
command_data parse_command(int argc, char **argv, int *opt);

/* Executes a command */
int command(command_data data);

// whether --wait was entered or not
extern int wait_flag;

#endif /* COMMAND_H */
