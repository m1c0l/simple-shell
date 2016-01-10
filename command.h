typedef struct {
  int in;
  int out;
  int err;
  char** argv;
} command_data;

/* Sets optind while consuming --command args */
int command(int argc, char **argv, int *opt);
