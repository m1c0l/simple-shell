#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include "filedesc.h"

/* Identifiers for the command line options
 * Starts from 1 because flag arguments use 0 */

enum Options {
  RDONLY = 1,
  WRONLY,
  RDWR,
  COMMAND,
  ABORT
};



/* Flags */
static int verbose_flag;

int main (int argc, char **argv) {
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"abort",   no_argument,       0, ABORT},
          {"rdonly",  required_argument, 0, RDONLY},
          {"wronly",  required_argument, 0, WRONLY},
          {"rdwr",    required_argument, 0, RDWR},
          {"command", no_argument, 0, COMMAND},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long_only(argc, argv, "",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            printf("flag set: %s\n", long_options[option_index].name);
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case RDONLY:
          printf("rdonly: %s\n", optarg);
          openFile(optarg, O_RDONLY);
          break; 

        case WRONLY:
          printf("wronly: %s\n", optarg);
          openFile(optarg, O_WRONLY);
          break;

        case RDWR:
          printf("rdwr: %s\n", optarg);
          break;

        case COMMAND: {
            /* Find index of the next argument starting with "--" */
            int arg_count = 0;
            for (int i = optind; i < argc && !(argv[i][0] == '-' &&
                argv[i][1] == '-'); i++)
              arg_count++;

            /* Need at least 4 arguments */
            if (arg_count < 4) {
              fprintf(stderr, "Not enough arguments for --command\n");
            }

            struct {
              int in;
              int out;
              int err;
              char** argv;
            } cmd_data;

            /* first 3 arguments are input, output, and error */
            cmd_data.in = atoi(argv[optind++]);
            cmd_data.out = atoi(argv[optind++]);
            cmd_data.err = atoi(argv[optind++]);

            /* size of command's argv */
            int argv_size = arg_count - 3;

            /* allocate space for argv, with a null terminator */
            cmd_data.argv = (char**)malloc(sizeof(char*) * (argv_size+1));
            cmd_data.argv[argv_size] = NULL;

            /* assign arguments to the command's argv */
            for (int i = 0; i < argv_size; i++) {
              cmd_data.argv[i] = argv[optind];
              optind++;
            }

            /* print the command's data */
            printf("in: %d\n", cmd_data.in);
            printf("out: %d\n", cmd_data.out);
            printf("err: %d\n", cmd_data.err);
            for (int i = 0; cmd_data.argv[i] != NULL; i++)
              printf("argv[%d]: %s\n", i, cmd_data.argv[i]);
          }
          break;

        case ABORT:
          {
            int *p = 0;
            *p = 0;
          }
          break;

        default:
          abort ();
        }
    }

  /* Instead of reporting ‘--verbose’
     and ‘--brief’ as they are encountered,
     we report the final status resulting from them. */
  if (verbose_flag)
    puts ("verbose flag is set");

  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }

  exit (0);
}
