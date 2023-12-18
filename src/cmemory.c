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

void _print_block(cmemory_block_t* block, size_t offset) {
  printf("START BLOCK -------- [\n");
  printf("[addr = %p]\n", (void*)block);
  printf("[size = %lu bytes    ]\n", block->size);
  printf("[end  = %p]\n",
         (char*)block + block->size + sizeof(cmemory_block_t) + offset);
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
  if (!head.next) {
    if (_init_list() == _CM_ALLOC_FAIL) {
      return NULL;
    }
  }

  // TODO: move this to a separate function so we don't write code twice
  const size_t request_size = size + sizeof(cmemory_block_t) + (_CM_ALIGN - 1);
  cmemory_block_t* block = head.next;
  while (block) {
    if (block->size >= request_size) {
      char* raw_ptr = (char*)block + sizeof(cmemory_block_t);
      char* aligned_ptr = (char*)_align(raw_ptr);
      const size_t offset = (size_t)aligned_ptr - (size_t)raw_ptr;

      char* next_addr = aligned_ptr + size;
      cmemory_block_t* next = (cmemory_block_t*)next_addr;
      next->size = block->size - size - sizeof(cmemory_block_t) - offset;
      next->next = NULL;

      block->next = next;
      block->size = size;
      _print_block(block, offset);
      _delete_block(block);

      return aligned_ptr;
    }
    block = block->next;
  }

  // TODO: try extending the volume here

  // TODO: search again

  printf("No available space found!\n");
  return NULL;
}

void cm_free(void* ptr) { printf("TODO: free %p\n", ptr); }
