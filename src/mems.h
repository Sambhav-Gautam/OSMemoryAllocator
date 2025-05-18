#ifndef MEMS_H
#define MEMS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct sub_t {
  void *mem_start_addr;
  size_t mem_size;
  void *mem_end_addr;
  uintptr_t SVA;
  uintptr_t EVA;
  int is_hole;
  struct sub_t *next;
  struct sub_t *prev;
} sub_t;

typedef struct main_t {
  void *mem_start_addr;
  uintptr_t SVA;
  uintptr_t EVA;
  void *current;
  size_t mem_size;
  size_t remaining;
  struct main_t *prev;
  struct main_t *next;
  sub_t sub_chain;
} main_t;

// External variable declarations
extern main_t *head;
extern sub_t *tail;
extern void *mems_start;
extern uintptr_t starting;
extern int mems_init_used;

// Function declarations
void mems_init(void);
void *mems_malloc(size_t size);
void mems_print_stats(void);
void mems_finish(void);
void mems_free(void *ptr);
void *mems_get(void *v_ptr);

#endif // MEMS_H
