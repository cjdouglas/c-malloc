#ifndef CMEMORY_H
#define CMEMORY_H

#include <stddef.h>

void* cm_malloc(size_t size);
void cm_free(void* ptr);

#endif  // CMEMORY_H
