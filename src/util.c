#include <string.h>

int is_not_option(char* s) {
  return strncmp(s, "--", 2);
}
