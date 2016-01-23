#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>

void initStream(void);
void endStream(void);

/* replace standard streams with these file numbers */
int set_streams(int in, int out, int err);
/* get stdin, stdout, stderr by passing in 0, 1, or 2 */
int getStream(int fd);
/* returns a FILE* pointing to stderr */
FILE *getStderrFile(void);
/* set 0, 1, and 2 back to standard streams */
int reset_streams(void);

#endif /* STREAM_H */
