#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* Identifiers for the command line options
 * Starts from 1 because flag arguments use 0 */

enum Options {
  RDONLY = 1,
  WRONLY,
  RDWR
};

void wronly(char* filename) {
  FILE *fp = fopen(filename, "w");
  if(fp == NULL) {
    fprintf(stderr, "wronly error\n");
    exit(1);
  }
  fprintf(fp, "Writing to wronly file\n");
  fclose(fp);
}

/* Flags */
static int verbose_flag;
static int abort_flag;

int main (int argc, char **argv) {
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          {"abort",   no_argument,       &abort_flag, 1},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"rdonly",  required_argument, 0, RDONLY},
          {"wronly",  required_argument, 0, WRONLY},
          {"rdwr",    required_argument, 0, RDWR},
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
          FILE *inFile = fopen(optarg, "r");
          if (!inFile) {
            fprintf(stderr, "Error opening file for input: %s", optarg);
            exit(0);
          }
          int readChar;
          while ((readChar = fgetc(inFile)) != EOF) {
            printf("%c", readChar);
          }
          fclose(inFile);
          break; 

        case WRONLY:
          printf("wronly: %s\n", optarg);
          wronly(optarg);
          break;

        case RDWR:
          printf("rdwr: %s\n", optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
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
