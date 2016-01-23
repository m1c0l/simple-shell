#ifndef UTIL_H
#define UTIL_H

#include <string.h>

/* returns true if string does not with "--" */
int is_not_option(char* s);

/* converts a string to a file descriptor int */
long get_file_desc(char *str);

#endif /* UTIL_H */
