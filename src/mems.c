#include "mems.h"

// Global variable definitions
main_t *head = NULL;
sub_t *tail = NULL;
void *mems_start = NULL;
uintptr_t starting = 1000;
int mems_init_used = 0;

void mems_init(void) {
  head = NULL;
  tail = NULL;
  mems_start = (void *)starting;
  mems_init_used = 1;
}

void *mems_malloc(size_t size) {
  if (size <= 0) {
    fprintf(stderr, "Error: Insufficient size. Size must be greater than zero.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  } else if (mems_init_used == 0) {
    fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  }

  size_t allocate = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

  if (head == NULL) {
    main_t *new_main = (main_t *)mmap(NULL, allocate + sizeof(main_t), PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_main == MAP_FAILED) {
      perror("Memory allocation failed");
      return NULL;
    }

    new_main->mem_start_addr = (char *)new_main + sizeof(main_t);
    new_main->current = new_main->mem_start_addr;
    new_main->SVA = starting;
    new_main->EVA = starting + allocate - 1;
    new_main->mem_size = allocate;
    new_main->remaining = allocate;
    new_main->prev = NULL;
    new_main->next = NULL;
    head = new_main;

    new_main->sub_chain.mem_start_addr = new_main->mem_start_addr;
    new_main->sub_chain.mem_end_addr =
        new_main->mem_start_addr + new_main->mem_size - 1;

    new_main->sub_chain.SVA = new_main->SVA;
    new_main->sub_chain.EVA = new_main->SVA + size - 1;

    new_main->sub_chain.mem_size = size;
    new_main->sub_chain.is_hole = 0;

    if (allocate > size) {
      sub_t *new_hole =
          (sub_t *)((char *)new_main->sub_chain.mem_start_addr + size);
      new_hole->mem_start_addr = (char *)new_main->sub_chain.mem_start_addr + size;
      new_hole->mem_end_addr = new_main->mem_start_addr + allocate - 1;

      new_hole->SVA = new_main->sub_chain.EVA + 1;
      new_hole->EVA = new_main->SVA + allocate - 1;

      new_hole->mem_size = allocate - size;
      new_hole->is_hole = 1;

      new_hole->prev = &new_main->sub_chain;
      new_hole->next = NULL;
      new_main->sub_chain.next = new_hole;
    }

    new_main->remaining -= size;
    new_main->current = new_main->sub_chain.mem_start_addr + size;

    printf("Allocated: main[SVA=%lu, EVA=%lu], sub[SVA=%lu, EVA=%lu]\n",
           new_main->SVA, new_main->EVA,
           new_main->sub_chain.SVA, new_main->sub_chain.EVA);

    return (void *)new_main->SVA;
  } else {
    main_t *current_main = head;

    while (current_main != NULL) {
      sub_t *current_sub = &(current_main->sub_chain);

      while (current_sub != NULL) {
        if (current_sub->is_hole && current_sub->mem_size >= size) {
          if (current_sub->mem_size > size) {
            sub_t *new_hole = (sub_t *)(((char *)current_sub->mem_start_addr) + size);
            new_hole->mem_start_addr = (void *)((char *)(current_sub->mem_start_addr) + size);
            new_hole->mem_end_addr = (void *)(((char *)new_hole->mem_start_addr) + (current_sub->mem_size - size) - 1);

            new_hole->SVA = current_sub->SVA + size;
            new_hole->EVA = current_sub->EVA;

            new_hole->mem_size = current_sub->mem_size - size;
            new_hole->is_hole = 1;

            new_hole->prev = current_sub;
            new_hole->next = current_sub->next;

            current_sub->mem_size = size;
            current_sub->EVA = current_sub->SVA + size - 1;
            current_sub->next = new_hole;
          }
          current_sub->is_hole = 0;

          current_main->remaining -= size;
          current_main->current = current_sub->mem_end_addr + 1;

          printf("Reused hole: main[SVA=%lu, EVA=%lu], sub[SVA=%lu, EVA=%lu]\n",
                 current_main->SVA, current_main->EVA,
                 current_sub->SVA, current_sub->EVA);

          return (void *)current_sub->SVA;
        }

        current_sub = current_sub->next;
      }

      if (current_main->next == NULL) {
        size_t new_allocate = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
        main_t *new_main =
            (main_t *)mmap(NULL, new_allocate + sizeof(main_t), PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (new_main == MAP_FAILED) {
          perror("Memory allocation failed");
          return NULL;
        }

        new_main->mem_start_addr = (char *)new_main + sizeof(main_t);
        new_main->current = new_main->mem_start_addr;
        new_main->SVA = current_main->EVA + 1;
        new_main->EVA = new_main->SVA + new_allocate - 1;
        new_main->mem_size = new_allocate;
        new_main->remaining = new_allocate;

        new_main->next = current_main->next;
        new_main->prev = current_main;
        current_main->next = new_main;

        new_main->sub_chain.mem_start_addr = new_main->mem_start_addr;
        new_main->sub_chain.mem_end_addr =
            new_main->mem_start_addr + new_main->mem_size - 1;

        new_main->sub_chain.SVA = new_main->SVA;
        new_main->sub_chain.EVA = new_main->SVA + size - 1;

        new_main->sub_chain.mem_size = size;
        new_main->sub_chain.is_hole = 0;

        if (new_allocate > size) {
          sub_t *new_hole =
              (sub_t *)((char *)new_main->sub_chain.mem_start_addr + size);
          new_hole->mem_start_addr = (char *)new_main->sub_chain.mem_start_addr + size;
          new_hole->mem_end_addr = new_main->mem_start_addr + new_allocate - 1;

          new_hole->SVA = new_main->sub_chain.EVA + 1;
          new_hole->EVA = new_main->SVA + new_allocate - 1;

          new_hole->mem_size = new_allocate - size;
          new_hole->is_hole = 1;

          new_hole->prev = &new_main->sub_chain;
          new_hole->next = NULL;
          new_main->sub_chain.next = new_hole;
        }

        new_main->remaining -= size;
        new_main->current = new_main->sub_chain.mem_start_addr + size;

        printf("Allocated new: main[SVA=%lu, EVA=%lu], sub[SVA=%lu, EVA=%lu]\n",
               new_main->SVA, new_main->EVA,
               new_main->sub_chain.SVA, new_main->sub_chain.EVA);

        return (void *)new_main->SVA;
      }

      current_main = current_main->next;
    }
  }
  return NULL;
}

void mems_print_stats(void) {
  if (mems_init_used == 0) {
    fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  }
  main_t *current = head;
  int total_pages_used = 0;
  size_t total_space_unused = 0;
  int main_chain_length = 0;
  int sub_chain_lengths[100];

  printf("MeMS Stats:\n");

  while (current != NULL) {
    printf("MAIN[%lu:%lu] -> ", current->SVA, current->EVA);

    sub_t *sub_current = &(current->sub_chain);
    int sub_chain_length = 0;

    while (sub_current != NULL) {
      printf("<%s>[SVA:%lu:EVA:%lu] <-> ",
             sub_current->is_hole ? "H" : "P", sub_current->SVA,
             sub_current->EVA);
      if (sub_current->is_hole) {
        total_space_unused += sub_current->mem_size;
      }
      sub_current = sub_current->next;
      sub_chain_length++;
    }

    sub_chain_lengths[main_chain_length] = sub_chain_length;
    printf("<NULL>\n");
    main_chain_length++;
    total_pages_used += current->mem_size / PAGE_SIZE;
    current = current->next;
  }

  printf("Page used: %d\n", total_pages_used);
  printf("Space unused: %zu\n", total_space_unused);
  printf("Main Chain Length: %d\n", main_chain_length);
  printf("Sub-chain Length array: ");

  for (int i = 0; i < main_chain_length; i++) {
    printf("%d ", sub_chain_lengths[i]);
  }
  printf("\n");
}

void mems_finish(void) {
  if (mems_init_used == 0) {
    fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  }

  main_t *current = head;
  while (current != NULL) {
    main_t *temp = current;
    current = current->next;
    if (munmap(temp, temp->mem_size + sizeof(main_t)) == -1) {
      perror(".......munmap failed......");
    }
  }

  head = NULL;
  tail = NULL;
  mems_start = NULL;
  mems_init_used = 0;
  printf("..........Successfully UnMapped the Memory without Errors .........\n");
}

void mems_free(void *ptr) {
  if (mems_init_used == 0) {
    fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  }
  main_t *current_main = head;

  while (current_main != NULL) {
    sub_t *current_sub = &(current_main->sub_chain);

    while (current_sub != NULL) {
      if (current_sub->SVA == (uintptr_t)ptr && !current_sub->is_hole) {
        current_sub->is_hole = 1;
        current_main->remaining += current_sub->mem_size;

        // Merge with next sub-chain if it's a hole
        sub_t *next_sub = current_sub->next;
        if (next_sub != NULL && next_sub->is_hole) {
          current_sub->mem_size += next_sub->mem_size;
          current_sub->mem_end_addr = next_sub->mem_end_addr;
          current_sub->EVA = next_sub->EVA;
          current_sub->next = next_sub->next;
          if (next_sub->next != NULL) {
            next_sub->next->prev = current_sub;
          }
        }

        // Update current pointer
        if (current_sub->next != NULL) {
          current_main->current = current_sub->next->mem_start_addr;
        } else {
          current_main->current = current_sub->mem_start_addr;
        }
        printf("Freed: sub[SVA=%lu, EVA=%lu]\n", current_sub->SVA, current_sub->EVA);
        return;
      }
      current_sub = current_sub->next;
    }
    current_main = current_main->next;
  }
  fprintf(stderr, "Invalid MeMS Virtual address for mems_free: %lu\n", (uintptr_t)ptr);
}

void *mems_get(void *v_ptr) {
  main_t *current_main = head;

  printf("mems_get: Checking v_ptr=%lu\n", (uintptr_t)v_ptr);
  while (current_main) {
    printf("  main[SVA=%lu, EVA=%lu]\n", current_main->SVA, current_main->EVA);
    if ((uintptr_t)v_ptr >= current_main->SVA &&
        (uintptr_t)v_ptr <= current_main->EVA) {
      printf("  Found: returning %p\n", current_main->mem_start_addr);
      return current_main->mem_start_addr;
    }
    current_main = current_main->next;
  }
  printf("  Not found: returning NULL\n");
  return NULL;
}

int main(int argc, char const *argv[]) {
  mems_init();
  int *ptr[10];

  printf("\n------- Allocated virtual addresses [mems_malloc] -------\n");
  for (int i = 0; i < 10; i++) {
    ptr[i] = (int *)mems_malloc(sizeof(int) * 250);
    printf("Virtual address: %lu\n", (unsigned long)ptr[i]);
  }
  mems_print_stats();

  printf("\n------ Assigning value to Virtual address [mems_get] -----\n");
  int *phy_ptr = (int *)mems_get(&ptr[0][1]);
  phy_ptr[0] = 200;
  int *phy_ptr2 = (int *)mems_get(&ptr[0][0]);
  printf("Virtual address: %lu\tPhysical Address: %lu\n", (unsigned long)ptr[0], (unsigned long)phy_ptr2);
  printf("Value written: %d\n", *phy_ptr2);

  printf("\n--------- Printing Stats [mems_print_stats] --------\n");
  mems_print_stats();

  printf("\n--------- Freeing up the memory [mems_free] --------\n");
  mems_free(ptr[3]);
  mems_print_stats();
  ptr[3] = (int *)mems_malloc(sizeof(int) * 250);
  mems_print_stats();

  printf("\n--------- Unmapping all memory [mems_finish] --------\n\n");
  mems_finish();
  return 0;
}
