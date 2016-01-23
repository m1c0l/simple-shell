#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int is_not_option(char* s) {
  return strncmp(s, "--", 2);
}

int get_file_desc(char *str) {
  char *endptr;
  int val = strtol(str, &endptr, 10);
  if ((errno == ERANGE && (val == INT_MAX || val == INT_MAX))
          || (errno != 0 && val == 0)) { // out of range
      fprintf(stderr, "Bad file descriptor %s\n", str);
      perror("strtol");
      return -1;
  }
  if (endptr == str) { // no digits found
      fprintf(stderr, "Bad file descriptor: no digits found in %s\n", str);
      return -1;
  }
  if (*endptr != '\0') { // letters found after #
      fprintf(stderr, "Bad file descriptor: letters found in %s\n", str);
      return -1;
  }
  if (val < 0) { // negative fd
    fprintf(stderr, "Bad file descriptor: negative # in %s\n", str);
    return -1;
  }
  return val;
}
