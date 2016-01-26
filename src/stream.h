#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>
#include "filedesc.h"
#include "command.h"

/* replace standard streams with these file numbers */
int set_streams(int in, int out, int err);

/* Returns 1 and reports an error if files are not usable as streams */
int invalid_files(file inf, file outf, file errf, command_data cmd_data);

#endif /* STREAM_H */
