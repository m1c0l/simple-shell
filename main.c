#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include "filedesc.h"
#include "command.h"



int main (int argc, char **argv) {
  /* Identifiers for the command line options
   * Starts from 1 because flag arguments use 0 */

  enum Options {
    VERBOSE = 1,
    RDONLY,
    WRONLY,
    RDWR,
    COMMAND,
    ABORT
  };


  static int verbose_flag;

  int commandReturn = 0;

  int c;

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
        for (int i = optind; i < argc &&
            !(argv[i][0] == '-' && argv[i][1] == '-'); i++) {
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
        {
          char *nextArg = argv[optind];
          if (!(nextArg[0] == '-' && nextArg[1] == '-')) {
            if (!commandReturn) {
              commandReturn = 1;
            }
          }
          openFile(optarg, O_RDONLY);
        }
          break; 

        case WRONLY:
          {
            char *nextArg = argv[optind];
            if (!(nextArg[0] == '-' && nextArg[1] == '-')) {
              fprintf(stderr, "Extra argument for %s: %s\n", long_options[option_index].name, argv[optind]);
              if (!commandReturn) {
                commandReturn = 1;
              }
            }
            openFile(optarg, O_WRONLY);
          }
          break;

        case RDWR:
          
          break;

        case COMMAND:
          { 
            int ret;
            ret = command(argc, argv, &optind);
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

  return commandReturn;
}
