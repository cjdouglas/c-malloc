#include "cmemory.h"

#include <stdio.h>
#include <sys/mman.h>

#define _CM_ALIGN 8
#define _CM_DEFAULT_CHUNK 4096
#define _CM_EXPAND_FAIL 0
#define _CM_EXPAND_SUCCESS 1
#define _CM_META_SIZE 16

// 16 byte struct for alignment purposes
typedef struct _cmemory_block {
  size_t size;
  struct _cmemory_block* next;
} cmemory_block_t;

cmemory_block_t head = {.size = 0, .next = NULL};

void _print_block(cmemory_block_t* block) {
  void* start = (void*)block;
  void* end = (void*)((char*)block + _CM_META_SIZE + block->size);

  printf("START BLOCK -------- [\n");
  printf("[addr = %p]\n", start);
  printf("[size = %lu bytes    ]\n", block->size);
  printf("[end  = %p]\n", end);
  printf("] ---------  END BLOCK\n\n");
}

void* _align(void* base_ptr) {
  const size_t offset = (size_t)base_ptr + _CM_ALIGN - 1;
  const size_t mask = ~(_CM_ALIGN - 1);
  return (void*)(offset & mask);
}

void _delete_block(cmemory_block_t* block) {
  cmemory_block_t* temp = &head;
  while (temp->next && temp->next != block) {
    temp = temp->next;
  }

  if (temp->next == block) {
    temp->next = block->next;
  }
}

// void _restore_block(cmemory_block_t* block) {}

// This function expands our heap by _CM_DEFAULT_CHUNK + request size.
int _expand(size_t size) {
  void* raw_block = mmap(NULL, _CM_DEFAULT_CHUNK + size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON, -1, 0);

  if (raw_block == MAP_FAILED) {
    return _CM_EXPAND_FAIL;
  }

  // Align the returned block so the first allocation is also aligned
  cmemory_block_t* block = _align(raw_block);
  const size_t offset = (size_t)block - (size_t)raw_block;
  block->size = _CM_DEFAULT_CHUNK + size - _CM_META_SIZE - offset;
  block->next = NULL;

  cmemory_block_t* current = &head;
  while (current->next) {
    current = current->next;
  }
  current->next = block;

  return _CM_EXPAND_SUCCESS;
}

void* _find_block(size_t size) {
  const size_t request_size = size + _CM_META_SIZE + (_CM_ALIGN - 1);
  cmemory_block_t* block = head.next;

  while (block) {
    if (block->size < request_size) {
      block = block->next;
      continue;
    }

    // Retrieve address of user memory region
    char* ptr = (char*)block + _CM_META_SIZE;

    // Create & align next block
    char* next_block = ptr + size;
    char* aligned_block = _align(next_block);
    const size_t offset = (size_t)aligned_block - (size_t)next_block;

    cmemory_block_t* next = (cmemory_block_t*)aligned_block;
    next->size = block->size - size - _CM_META_SIZE - offset;
    next->next = NULL;

    // Update & remove selected block
    block->next = next;
    block->size = size;
    _print_block(block);
    _delete_block(block);

    return ptr;
  }

  return NULL;
}

void* cm_malloc(size_t size) {
  void* ptr = _find_block(size);
  if (ptr) {
    return ptr;
  }

  if (_expand(size) == _CM_EXPAND_FAIL) {
    return NULL;
  }

  ptr = _find_block(size);
  if (ptr) {
    return ptr;
  }

  return NULL;
}

void cm_free(void* ptr) {
  char* raw_addr = (char*)ptr - _CM_META_SIZE;
  cmemory_block_t* block = (cmemory_block_t*)raw_addr;
  _print_block(block);

  // TODO: add this block back to the list, and coalesce if possible
}
