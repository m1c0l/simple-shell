#ifndef FILEDESC_H
#define FILEDESC_H

#include <stdio.h>

typedef struct {
  int fd;
  int oflag;
} file;

void initFileDesc();
void endFileDesc();
int openFile(char* filename, int oflag);

file getFile(int index);

/* get stdin, stdout, stderr by passing in 0, 1, or 2 */
int getStream(int fd);
/* returns a FILE* pointing to stderr */
FILE *getStderrFile();
/* set 0, 1, and 2 back to standard streams */
int reset_streams();

#endif /* FILEDESC_H */
