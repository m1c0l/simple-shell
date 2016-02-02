#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main () {
  int i;
  int urandom = open("/dev/urandom", O_RDONLY);
  char buf[100];
  // system time
  for (i = 0; i < 200; i++) {
    read(urandom, &buf, 100);
  }

  for (i = 0; i < 30000000; i++)
    continue;

  return 0;
}
