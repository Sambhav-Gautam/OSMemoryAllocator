#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

typedef struct sub_t {
  void *mem_start_addr;
  size_t mem_size;
  void *mem_end_addr;
  int SVA;
  int EVA;
  int is_hole;
  struct sub_t *next;
  struct sub_t *prev;
} sub_t;

typedef struct main_t {
  void *mem_start_addr;
  int SVA;
  int EVA;
  void *current;
  size_t mem_size;
  size_t remaining;
  struct main_t *prev;
  struct main_t *next;
  sub_t sub_chain;
} main_t;

main_t *head ;
sub_t *tail = NULL;
void *mems_start = NULL;
int starting = 1000;
int mems_init_used  = 0;

void mems_init() {
  head = NULL;
  tail = NULL;
  mems_start = (void *)starting;
  mems_init_used =1;
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

  size_t allocate = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE + sizeof(main_t);


  if (head == NULL) {
    main_t *new_main = (main_t *)mmap(NULL, allocate, PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);


    allocate -= sizeof(main_t);

    if (new_main == MAP_FAILED) {
      perror("Memory allocation failed");
      return NULL;
    }

    new_main->mem_start_addr = new_main;
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
        new_main->mem_start_addr + new_main->mem_size  - 1;

    // Virtual Mapping
    new_main->sub_chain.SVA = new_main->SVA;
    new_main->sub_chain.EVA = new_main->SVA + size - 1;
    // Virtual Mapping

    new_main->sub_chain.mem_size = new_main->mem_size;
    new_main->sub_chain.is_hole =
        0; 

    
    if (allocate > size) {
      sub_t *new_hole =
          (sub_t *)((char *)new_main->sub_chain.mem_start_addr + size);
      new_hole->mem_start_addr = new_main->sub_chain.mem_end_addr + 1;
      new_hole->mem_end_addr = new_hole->mem_start_addr + (allocate - size) - 1;

      // Virtual Mapping
      new_hole->SVA = new_main->sub_chain.EVA + 1;
      new_hole->EVA = new_hole->SVA + (allocate - size) - 1;
      // Virtual Mapping

      new_hole->mem_size = allocate - size;
      new_hole->is_hole = 1; // The remaining space is a hole

      new_hole->prev =
          &new_main
               ->sub_chain;  // Set the 'prev' pointer to the process sub-node
      new_hole->next = NULL; // There's no sub-node after the hole
      new_main->sub_chain.next = new_hole;
      // new_main->sub_chain.prev = new_main;
    }

    
    new_main->remaining -= size;
    new_main->current = new_main->sub_chain.mem_start_addr + size;

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


            // Virtual Mapping
            new_hole->SVA = current_sub->SVA + size;
            // new_hole->SVA = current_sub->EVA + 1;
            new_hole->EVA = current_sub->EVA;
            // new_hole->EVA = new_hole->SVA + (current_sub->mem_size - size) -
            // 1;
            //  Virtual Mapping

            new_hole->mem_size = current_sub->mem_size - size;
            new_hole->is_hole = 1; 

            new_hole->prev =
                current_sub; // Set the 'prev' pointer to the process sub-node
            new_hole->next =
                current_sub->next; // Link the new sub-node to the next one

            current_sub->mem_size = size;
            
            current_sub -> EVA = current_sub -> SVA +size -1; 
            current_sub->next =
                new_hole; 
          }
          current_sub -> is_hole =0; 
          
          current_main->remaining -= size;
          current_main->current = current_sub->mem_end_addr + 1;

          return current_sub->SVA;
        }

        current_sub = current_sub->next;
      }

      if (current_main->next == NULL) {
       
        size_t new_allocate = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE ;
        new_allocate += sizeof(main_t); 
        main_t *new_main =
            (main_t *)mmap(NULL, new_allocate, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        new_allocate -=sizeof(main_t);

        if (new_main == MAP_FAILED) {
          perror("Memory allocation failed");
          return NULL;
        }

        new_main->mem_start_addr = new_main;
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
            new_main->mem_start_addr + new_main->mem_size - 1; //

        // Virtual Mapping
        new_main->sub_chain.SVA = new_main->SVA;
        new_main->sub_chain.EVA = new_main->SVA + size - 1;
        // Virtual Mapping

        new_main->sub_chain.mem_size = new_main->mem_size;
        new_main->sub_chain.is_hole =
            0; 

        
        if (new_allocate > size) {
          sub_t *new_hole =
              (sub_t *)((char *)new_main->sub_chain.mem_start_addr + size);
          new_hole->mem_start_addr =
              new_main->sub_chain.mem_end_addr +
              1; // changed from sub_chain.mem_end_addr +1
          new_hole->mem_end_addr = new_hole->mem_start_addr +
                                   (new_allocate - size) -
                                   1; // changed fromnew_hole->mem_start_addr +
                                      // (new_allocate - size) - 1

          // Virtual Mapping
          new_hole->SVA = new_main->sub_chain.EVA + 1;
          new_hole->EVA = new_hole->SVA + (new_allocate - size) - 1;
          // Virtual Mapping

          new_hole->mem_size = new_allocate - size;
          new_hole->is_hole = 1; // The remaining space is a hole

          new_hole->prev = &new_main->sub_chain; 
          new_hole->next = NULL;
          new_main->sub_chain.next = new_hole; 
        }

        // Update the remaining and current pointers in the new main node.
        new_main->remaining -= size;
        new_main->current = new_main->sub_chain.mem_start_addr + size;

        return (void *)new_main->SVA;
      }

      current_main = current_main->next;
    }
  }
}

void mems_print_stats() {
    if (mems_init_used == 0) {
      fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
      fprintf(stderr, "Program exited ...\n");
      exit(EXIT_FAILURE);
    }
    main_t *current = head;
    int total_pages_used = 0;
    size_t total_space_unused = 0;
    int main_chain_length = 0;
    int sub_chain_lengths[100]; // Assuming a maximum of 100 sub-chains

    printf("MeMS Stats:\n");

    while (current != NULL) {
        printf("MAIN[%d:%d] -> ", current->SVA, current->EVA);

        sub_t *sub_current = &current->sub_chain;
        int sub_chain_length = 0;

        while (sub_current != NULL) {
        printf("<%s>[SVA:%d:EVA:%d] <-> ",
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

void mems_finish() {
  if (mems_init_used == 0) {
    fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
    fprintf(stderr, "Program exited ...\n");
    exit(EXIT_FAILURE);
  }

   main_t *current = head;
   while (current != NULL) {
       main_t *temp = current;
       current = current->next;
       if (munmap(temp, temp->mem_size) == -1) {
          
           perror(".......munmap failed......");
           
       }
   }

   head = NULL;
   tail = NULL;
   mems_start = NULL;
   mems_init_used =0;
   printf("..........Successfully UnMapped the Memory without Errors .........\n");
}


void mems_free(void* ptr) {
    if (mems_init_used == 0) {
        fprintf(stderr, "Error: Memory system not initialized. Call mems_init() first.\n");
        fprintf(stderr, "Program exited ...\n");
        exit(EXIT_FAILURE);
    }
    main_t* current_main = head;

    while (current_main != NULL) {
        sub_t *current_sub = &(current_main->sub_chain);

        while (current_sub != NULL) {
            if (current_sub->SVA == ptr) {
                current_sub->is_hole = 1;
                current_main->remaining += current_sub->mem_size;


                sub_t* next_sub = current_sub->next;
                if (next_sub != NULL && next_sub->is_hole) {
                    current_sub->mem_size += next_sub->mem_size;
                    current_sub->mem_end_addr = next_sub->mem_end_addr;
                    current_sub->EVA = next_sub->EVA;
                    current_sub->next = next_sub->next;
                    if (next_sub->next != NULL) {
                        next_sub->next->prev = current_sub;
                    }
                }

                sub_t* prev_sub = current_sub->prev;
                if (prev_sub != NULL && prev_sub->is_hole) {
                    prev_sub->mem_size += current_sub->mem_size;
                    prev_sub->mem_end_addr = current_sub->mem_end_addr;
                    prev_sub->EVA = current_sub->EVA;
                    prev_sub->next = current_sub->next;
                    if (current_sub->next != NULL) {
                        current_sub->next->prev = prev_sub;
                    }
                }

                if (prev_sub != NULL && prev_sub->is_hole && current_sub->is_hole) {
                    prev_sub->mem_size += current_sub->mem_size;
                    prev_sub->mem_end_addr = current_sub->mem_end_addr;
                    prev_sub->EVA = current_sub->EVA;
                    prev_sub->next = current_sub->next;
                    if (current_sub->next != NULL) {
                        current_sub->next->prev = prev_sub;
                    }
                }

                if (current_sub->next != NULL) {
                    current_main->current = current_sub->next->mem_start_addr;
                } else {
                    current_main->current = current_sub->mem_start_addr;
                }
                return;
            }

            current_sub = current_sub->next;
        }

        current_main = current_main->next;
    }

    perror("Invalid MeMS Virtual address for mems_free\n");
}



void *mems_get(void *v_ptr) {
    main_t *current_main = head;

    while (current_main) {
        if ((unsigned long)v_ptr >= (unsigned long)current_main->SVA &&
            (unsigned long)v_ptr <= (unsigned long)current_main->EVA) {
            return current_main->mem_start_addr;
        }
        current_main = current_main->next;
    }

    return NULL; 
}
