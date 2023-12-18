#include <stdio.h>

#include "cmemory.h"

typedef struct _large_struct {
  int arr[64];
  char str[32];
} large_struct_t;

int main() {
  // printf("sizeof large_struct_t = %lu\n", sizeof(large_struct_t));

  int* i = cm_malloc(sizeof *i);
  if (i) {
    *i = 42;
    // printf("i pointer: %p\n", i);
    // printf("i value  : %d\n", *i);
  }

  int* j = cm_malloc(sizeof *j);
  if (j) {
    *j = 43;
    // printf("j pointer: %p\n", j);
    // printf("j value  : %d\n", *j);
  }

  large_struct_t* lst = cm_malloc(sizeof *lst);
  if (lst) {
    lst->arr[0] = 53;
    lst->str[0] = 'a';
    // printf("lst pointer: %p\n", lst);
    // printf("arr[0] value  : %d\n", lst->arr[0]);
    // printf("str[0] value  : %c\n", lst->str[0]);
  }

  int* k = cm_malloc(sizeof *k);
  if (k) {
    *k = 69;
    // printf("k pointer: %p\n", k);
    // printf("k value  : %d\n", *k);
  }

  double* m = cm_malloc(sizeof *m);
  if (m) {
    *m = 3.5;
  }

  double* n = cm_malloc(sizeof *n);
  if (n) {
    *n = 3.7;
  }

  float* p = cm_malloc(sizeof *p);

  return 0;
}
