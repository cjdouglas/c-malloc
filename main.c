#include <stdio.h>

#include "cmemory.h"

typedef struct _large_struct {
  int arr[64];
  char str[32];
} large_struct_t;

typedef struct _smaller_struct {
  int arr[60];
  char str[29];
} smaller_struct_t;

int main() {
  // printf("sizeof large_struct_t = %lu\n", sizeof(large_struct_t));

  int* i = cm_malloc(sizeof *i);
  if (!i) {
    printf("i alloc failed\n");
  }

  int* j = cm_malloc(sizeof *j);
  if (!j) {
    printf("j alloc failed\n");
  }

  int* k = cm_malloc(sizeof *k);
  if (!k) {
    printf("k alloc failed\n");
  }

  large_struct_t* lst = cm_malloc(sizeof *lst);
  if (!lst) {
    printf("lst alloc failed\n");
  }

  smaller_struct_t* sst = cm_malloc(sizeof *sst);
  if (!sst) {
    printf("sst alloc failed\n");
  }

  const size_t n_bytes = 3420;
  char* large_block = cm_malloc(n_bytes * sizeof *large_block);
  if (!large_block) {
    printf("large_block alloc failed\n");
  }

  cm_free(lst);
  cm_free(sst);
  cm_free(i);
  cm_free(j);
  cm_free(k);
  cm_free(large_block);
  cm_dump_core();

  return 0;
}
