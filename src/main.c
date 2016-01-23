#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#define _GNU_SOURCE

#include "filedesc.h"
#include "command.h"
#include "stream.h"
#include "util.h"

/* Identifiers for the command line options
 * Starts from 1 because flag arguments use 0 */

enum Options {
  VERBOSE = 1,
  RDONLY,
  WRONLY,
  RDWR,
  APPEND,
  CLOEXEC,
  CREAT,
  DIRECTORY,
  DSYNC,
  EXCL,
  NOFOLLOW,
  NONBLOCK,
  RSYNC,
  SYNC,
  TRUNC,
  COMMAND,
  ABORT
};

static int verbose_flag;
int commandReturn = 0;
int file_oflags = 0;

static struct option long_options[] =
{
  {"verbose", no_argument,       0, VERBOSE},
  {"abort",   no_argument,       0, ABORT},
  {"append",   no_argument,       0, APPEND},
  {"cloexec",   no_argument,       0, CLOEXEC},
  {"creat",   no_argument,       0, CREAT},
  {"directory",   no_argument,       0, DIRECTORY},
  {"dsync",   no_argument,       0, DSYNC},
  {"excl",   no_argument,       0, EXCL},
  {"nofollow",   no_argument,       0, NOFOLLOW},
  {"nonblock",   no_argument,       0, NONBLOCK},
  {"rsync",   no_argument,       0, RSYNC},
  {"sync",   no_argument,       0, SYNC},
  {"trunc",   no_argument,       0, TRUNC},
  {"rdonly",  required_argument, 0, RDONLY},
  {"wronly",  required_argument, 0, WRONLY},
  {"rdwr",    required_argument, 0, RDWR},
  {"command", no_argument, 0, COMMAND},
  {0, 0, 0, 0}
};
/* getopt_long stores the option index here. */
int option_index = 0;

// optind is global so we don't need it in the args here
void parseOflags(int oflag) {
  // store the flag
  file_oflags |= oflag;

  if (oflag == O_RDONLY || oflag == O_WRONLY || oflag == O_RDWR) {
    // open the file
    int openStatus = openFile(optarg, file_oflags);
    if (openStatus) {
      if (!commandReturn) {
          commandReturn = 1;
        }
    }
    // reset the oflags
    file_oflags = 0;
  }
}


int main (int argc, char **argv) {
  initFileDesc(); // allocate file descriptor array
  initStream(); // create copies of standard streams

  int c;

  while (1)
    {
      c = getopt_long(argc, argv, "",
                       long_options, &option_index);
      
      /* Detect the end of the options. */
      if (c == -1)
        break;

      if (verbose_flag) {
        printf("--%s", long_options[option_index].name);
        if (optarg != NULL) {
          printf(" %s", optarg);
        }
        for (int i = optind; i < argc && is_not_option(argv[i]); i++) {
          printf(" %s", argv[i]);
        }
        printf("\n");
        fflush(stdout);
      }

      switch (c)
        {
        case VERBOSE:
          verbose_flag = 1;
          break;

        case RDONLY:
          parseOflags(O_RDONLY);
          break;

        case WRONLY:
          parseOflags(O_WRONLY);
          break;

        case RDWR:
          parseOflags(O_RDWR);
          break;

        case APPEND:
          parseOflags(O_APPEND);
          break;

        case CLOEXEC:
          parseOflags(O_CLOEXEC);
          break;

        case CREAT:
          parseOflags(O_CREAT);
          break;

        case DIRECTORY:
          parseOflags(O_DIRECTORY);
          break;

        case DSYNC:
          parseOflags(O_DSYNC);
          break;

        case EXCL:
          parseOflags(O_EXCL);
          break;

        case NOFOLLOW:
          parseOflags(O_NOFOLLOW);
          break;

        case NONBLOCK:
          parseOflags(O_NONBLOCK);
          break;

        case RSYNC:
          parseOflags(O_RSYNC);
          break;

        case SYNC:
          parseOflags(O_SYNC);
          break;

        case TRUNC:
          parseOflags(O_TRUNC);
          break;

        case COMMAND:
          { 
            command_data cmd_data = parse_command(argc, argv, &optind);
            int ret = command(cmd_data);
            if (ret > commandReturn) {
              commandReturn = ret;
            }
          }
          break;

        case ABORT:
          {
            // intentionally cause segfault
            int *p = 0;
            *p = 0;
          }
          break;

        case '?':
          // code for unrecognized options
          if (!commandReturn) {
            commandReturn = 1;
          }
          break;
        default:
          break;
        }
        if (optind < argc)
        {
          char *nextArg = argv[optind];
          if (is_not_option(nextArg)) {
            fprintf(stderr, "Extra argument for %s: %s\n", long_options[option_index].name, argv[optind]);
            if (!commandReturn) {
              commandReturn = 1;
            }
          }
        }
    }

  endFileDesc(); // free file descriptor array
  endStream(); // close standard stream copies

  return commandReturn;
}
