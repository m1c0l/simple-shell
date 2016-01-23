#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#define _GNU_SOURCE

#include "filedesc.h"
#include "command.h"
#include "util.h"



int main (int argc, char **argv) {
  initFileDesc(); // allocate file descriptor array

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

  int c;

  int file_oflags = 0;

  while (1)
    {
      static struct option long_options[] =
        {
          {"verbose", no_argument,       0, VERBOSE},
          {"abort",   no_argument,       0, ABORT},
          {"rdonly",  required_argument, 0, RDONLY},
          {"wronly",  required_argument, 0, WRONLY},
          {"rdwr",    required_argument, 0, RDWR},
          {"command", no_argument, 0, COMMAND},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

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
          {
            file_oflags |= O_RDONLY;
            int openStatus = openFile(optarg, file_oflags);
            if (openStatus) {
              if (!commandReturn) {
                  commandReturn = 1;
                }
            }
            file_oflags = 0;
          }
          break; 

        case WRONLY:
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
          {
            file_oflags |= O_WRONLY;
            int openStatus = openFile(optarg, file_oflags);
            if (openStatus) {
              if (!commandReturn) {
                  commandReturn = 1;
                }
            }
            file_oflags = 0;
          }
          break;

        case RDWR:
          
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
    }

  endFileDesc(); // free file descriptor array

  return commandReturn;
}