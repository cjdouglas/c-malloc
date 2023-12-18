#include "cmemory.h"

#include <stdio.h>
#include <sys/mman.h>

#define _CM_ALIGN 8
#define _CM_DEFAULT_CHUNK 4096
#define _CM_ALLOC_FAIL 0
#define _CM_ALLOC_SUCCESS 1

typedef struct _cmemory_block {
  size_t size;
  struct _cmemory_block* next;
} cmemory_block_t;

cmemory_block_t head = {.size = 0, .next = NULL};

void _print_block(cmemory_block_t* block) {
  printf("START BLOCK -------- [\n");
  printf("[addr = %p]\n", (void*)block);
  printf("[size = %lu bytes    ]\n", block->size);
  printf("] ---------  END BLOCK\n\n");
}

void* _align(void* base_ptr) {
  const size_t offset = (size_t)base_ptr + _CM_ALIGN - 1;
  const size_t mask = ~(_CM_ALIGN - 1);
  return (void*)(offset & mask);
}

// Removes the block from the free list
void _delete_block(cmemory_block_t* block) {
  cmemory_block_t* temp = &head;
  while (temp->next && temp->next != block) {
    temp = temp->next;
  }

  if (temp->next == block) {
    temp->next = block->next;
  }
}

int _init_list() {
  cmemory_block_t* chunk = mmap(NULL, _CM_DEFAULT_CHUNK, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANON, -1, 0);
  if (chunk == MAP_FAILED) {
    return _CM_ALLOC_FAIL;
  }
  chunk->size = _CM_DEFAULT_CHUNK - sizeof(cmemory_block_t);
  chunk->next = NULL;

  head.next = chunk;
  return _CM_ALLOC_SUCCESS;
}

void* cm_malloc(size_t size) {
  printf("searching for %lu bytes\n", size + sizeof(cmemory_block_t));
  if (!head.next) {
    if (_init_list() == _CM_ALLOC_FAIL) {
      return NULL;
    }
  }

  cmemory_block_t* block = head.next;
  while (block) {
    if (block->size >= size + sizeof(cmemory_block_t)) {
      // Get a pointer to the actual usable space
      void* ptr = (char*)block + sizeof(cmemory_block_t);

      // Find the address for the next metadata struct
      char* next_addr = (char*)block + size + sizeof(cmemory_block_t);

      // Create a new block in that location
      cmemory_block_t* next = (cmemory_block_t*)next_addr;
      next->size = block->size - size - sizeof(cmemory_block_t);
      next->next = NULL;

      // Set the final metadata of the block, and remove from the free list
      block->next = next;
      block->size = size;
      _delete_block(block);

      // Return the saved memory address
      return ptr;
    }
    block = block->next;
  }

  printf("No available space found!\n");
  return NULL;
}

void cm_free(void* ptr) {}
