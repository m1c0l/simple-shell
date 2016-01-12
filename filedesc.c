#include "filedesc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int g_fileDesc[100];
int g_currFileDesc = 0;

void openFile(char* filename, int oflag) {
  int fd = open(filename, oflag);
  if (fd == -1) {
  	fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
  	exit(1);
  }
  g_fileDesc[g_currFileDesc] = fd;
  g_currFileDesc++;
  // printf("fd: %d\n", fd);
}