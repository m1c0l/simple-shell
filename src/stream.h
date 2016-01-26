#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>

void initStream(void);
void endStream(void);

/* replace standard streams with these file numbers */
int set_streams(int in, int out, int err);

#endif /* STREAM_H */
