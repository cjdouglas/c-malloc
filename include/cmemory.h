#ifndef CMEMORY_H
#define CMEMORY_H

#include <stddef.h>

void* cm_malloc(size_t size);
void cm_free(void* ptr);
void cm_dump_core();

#endif  // CMEMORY_H
