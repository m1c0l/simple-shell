#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main () {
  int i;
  int urandom = open("/dev/urandom", O_RDONLY);
  char buf[1000];
  // system time
  for (i = 0; i < 1000; i++) {
    read(urandom, &buf, 1000);
  }

  for (i = 0; i < 30000000; i++)
    continue;

  return 0;
}
