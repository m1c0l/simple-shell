#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "signal_handler.h"

int getSignalNumber(char *str) {
  char *endptr;
  int val = strtol(str, &endptr, 10);
  if ((errno == ERANGE && (val == INT_MAX || val == INT_MAX))
          || (errno != 0 && val == 0)) { // out of range
      fprintf(stderr, "Bad signal number %s\n", str);
      perror("strtol");
      return -1;
  }
  if (endptr == str) { // no digits found
      fprintf(stderr, "Bad signal number: no digits found in %s\n", str);
      return -1;
  }
  if (*endptr != '\0') { // letters found after #
      fprintf(stderr, "Bad signal number: letters found in %s\n", str);
      return -1;
  }
  if (val < 0) { // negative fd
    fprintf(stderr, "Bad signal number: negative # in %s\n", str);
    return -1;
  }
  return val;
}

int ignoreSignal(char* optarg) {
	int N = getSignalNumber(optarg);
	if (signal(N, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "Signal: %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

int useDefaultSignal(char* optarg) {
	int N = getSignalNumber(optarg);
	if (signal(N, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "Signal: %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

int raiseAbortSignal() {
	if (raise(SIGSEGV)) {
		fprintf(stderr, "Couldn't send SIGSEGV signal\n");
		return 1;
	}
	return 0;
}