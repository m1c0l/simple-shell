#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <sys/time.h>

/* returns true if string does not with "--" */
int is_not_option(char* s);

/* converts a string to a file descriptor int */
int get_file_desc(char *str);

/* returns the time elapsed in microseconds */
int get_time_diff(struct timeval prev, struct timeval curr);

#endif /* UTIL_H */
