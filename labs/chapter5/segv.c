#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int64_t* p = NULL;
  puts("before invalid access");

  *p = 0;
  puts("after invalid access");

  return EXIT_SUCCESS;
}
