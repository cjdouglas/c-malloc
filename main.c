#include <stdio.h>

#include "cmemory.h"

int main() {
  int* i = cm_malloc(sizeof *i);
  if (i) {
    *i = 42;
    printf("i pointer: %p\n", i);
    printf("i value  : %d\n", *i);
  }

  int* j = cm_malloc(sizeof *j);
  if (j) {
    *j = 43;
    printf("j pointer: %p\n", j);
    printf("j value  : %d\n", *j);
  }

  return 0;
}
