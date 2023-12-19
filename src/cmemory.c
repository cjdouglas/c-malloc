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

  printf("--------------------------------\n");
  printf("[addr = %p]\n", start);
  printf("[size = %lu bytes    ]\n", block->size);
  printf("[end  = %p]\n", end);
  printf("--------------------------------\n");
}

void _dump_blocks() {
  cmemory_block_t* current = head.next;
  printf("START BLOCK DUMP\n");
  while (current) {
    _print_block(current);
    current = current->next;
  }
  printf("END BLOCK DUMP\n\n");
}

void* _align(void* base_ptr) {
  const ptrdiff_t offset = (ptrdiff_t)base_ptr + _CM_ALIGN - 1;
  const ptrdiff_t mask = ~(_CM_ALIGN - 1);
  return (void*)(offset & mask);
}

void _delete_block(cmemory_block_t* block) {
  cmemory_block_t* temp = &head;
  while (temp->next && temp->next != block) {
    temp = temp->next;
  }

  if (temp->next == block) {
    temp->next = block->next;
    block->next = NULL;
  }
}

// Coalesces the two blocks by merging `block` and `current`. The coalesced data
// will be stored in `block` and placed into the list.
void _coalesce(cmemory_block_t* block, cmemory_block_t* current) {
  cmemory_block_t* next = current->next;
  char* b_end = (char*)current + current->size + _CM_META_SIZE;
  char* a_start = (char*)block + _CM_META_SIZE;
  const size_t new_size = (size_t)b_end - (size_t)a_start;
  block->size = new_size;
  block->next = next;
}

void _try_coalesce() {
  cmemory_block_t* current = head.next;
  while (current && current->next) {
    ptrdiff_t dist = (ptrdiff_t)current->next - (ptrdiff_t)current;
    if (dist - current->size - _CM_META_SIZE >= _CM_ALIGN) {
      current = current->next;
      continue;
    }

    cmemory_block_t* next = current->next->next;
    char* end = (char*)current->next + current->next->size + _CM_META_SIZE;
    char* start = (char*)current + _CM_META_SIZE;
    const size_t new_size = (size_t)end - (size_t)start;
    current->size = new_size;
    current->next = next;
  }
}

void _reclaim_block(cmemory_block_t* block) {
  cmemory_block_t* prev = &head;
  cmemory_block_t* current = head.next;
  while (current) {
    ptrdiff_t dist = (ptrdiff_t)current - (ptrdiff_t)block;
    if (dist < 0) {
      prev = current;
      current = current->next;
      continue;
    }

    prev->next = block;
    block->next = current;
    return;
  }

  prev->next = block;
}

// This function expands our heap by _CM_DEFAULT_CHUNK + request size.
int _expand(size_t size) {
  void* raw_block = mmap(NULL, _CM_DEFAULT_CHUNK + _CM_ALIGN + size,
                         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (raw_block == MAP_FAILED) {
    return _CM_EXPAND_FAIL;
  }

  // Align the returned block so the first allocation is also aligned
  cmemory_block_t* block = _align(raw_block);
  const ptrdiff_t offset = (ptrdiff_t)block - (ptrdiff_t)raw_block;
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
  cmemory_block_t* block = head.next;

  while (block) {
    if (block->size < size) {
      block = block->next;
      continue;
    }

    // Retrieve address of user memory region
    char* ptr = (char*)block + _CM_META_SIZE;

    if (block->size == size) {
      _delete_block(block);
      return ptr;
    }

    // Create & align next block
    char* next_block = ptr + size;
    char* aligned_block = _align(next_block);
    const ptrdiff_t offset = (ptrdiff_t)aligned_block - (ptrdiff_t)next_block;

    cmemory_block_t* next = (cmemory_block_t*)aligned_block;
    next->size = block->size - size - _CM_META_SIZE - offset;
    next->next = block->next;

    // Update & remove selected block
    block->next = next;
    block->size = size;
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
  char* block = (char*)ptr - _CM_META_SIZE;
  _reclaim_block((cmemory_block_t*)block);
  _try_coalesce();
}

void cm_dump_core() { _dump_blocks(); }
