#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

typedef struct sub_t {
    void* mem_start_addr;
    size_t mem_size;
    void* mem_end_addr;
    int SVA;
    int EVA;
    int is_hole;
    struct sub_t* next;
    struct sub_t* prev;
} sub_t;

typedef struct main_t {
    void* mem_start_addr;
    int SVA;
    int EVA;
    void* current;
    size_t mem_size;
    size_t remaining;
    struct main_t* prev;
    struct main_t* next;
    sub_t sub_chain;
} main_t;

main_t* head = NULL;
sub_t* tail = NULL;
void* mems_start = NULL;
int starting = 1000;

void mems_init() {
    head = NULL;
    tail = NULL;
    mems_start = (void*)starting;
}

void* mems_malloc(size_t size) {
    if (size <= 0) {
        perror("Insufficient size. Size must be greater than zero.");
        return NULL;
    }

    size_t allocate = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

    if (head == NULL) {
        main_t* newnode = (main_t*)mmap(NULL, allocate, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        head = newnode;
        newnode->mem_start_addr = newnode;
        newnode->current = newnode->mem_start_addr;
        newnode->SVA = starting;
        newnode->EVA = starting + allocate - 1;
        newnode->mem_size = allocate;
        newnode->remaining = allocate; 
        newnode->prev = NULL;
        newnode->next = NULL;

        newnode->sub_chain.mem_start_addr = newnode->mem_start_addr;
        newnode->sub_chain.mem_end_addr = newnode->mem_start_addr + newnode->mem_size - 1;

        //Virtual Mapping 
        newnode -> sub_chain.SVA = newnode -> SVA;
        newnode -> sub_chain.EVA = newnode -> SVA + size -1;
        //Virtual Mapping

        newnode->sub_chain.mem_size = newnode->mem_size;
        newnode->sub_chain.is_hole = 0;  // The current sub-node is not a hole, it's occupied by a process

        // Create a sub-node for the remaining hole.
        newnode->sub_chain.next = (sub_t*)((char*)newnode->sub_chain.mem_start_addr + size);
        newnode->sub_chain.next->mem_start_addr = newnode->sub_chain.mem_end_addr + 1;
        newnode->sub_chain.next->mem_end_addr = newnode->sub_chain.next->mem_start_addr + (allocate - size) - 1;

        //Virtual Mapping
        newnode ->sub_chain.next->SVA = newnode -> sub_chain.EVA+1;
        newnode -> sub_chain.next->EVA = newnode ->sub_chain.EVA +allocate -size;
        //Virtual Mapping
        newnode->sub_chain.next->mem_size = allocate - size;
        newnode->sub_chain.next->is_hole = 1;  // The remaining space is a hole

        newnode->sub_chain.next->prev = &newnode->sub_chain;  // Set the 'prev' pointer to the process sub-node
        newnode->sub_chain.next->next = NULL;  // There's no sub-node after the hole

        // Update the remaining and current pointers.
        newnode->remaining -= size;
        newnode->current = newnode->sub_chain.next->mem_end_addr + 1;

        return (void*)newnode->SVA;
    } else {
        // Implement logic for reusing segments from the free list and other cases.
        // ...
    }
}

void mems_print_stats() {
    main_t* current = head;
    int total_pages_used = 0;
    size_t total_space_unused = 0;
    int main_chain_length = 0;
    int sub_chain_lengths[100]; // Assuming a maximum of 100 sub-chains

    printf("MeMS Stats:\n");

    while (current != NULL) {
        printf("MAIN[%d:%d] -> ", current->SVA, current->EVA);

        sub_t* sub_current = &current->sub_chain;
        int sub_chain_length = 0;

        while (sub_current != NULL) {
            printf("<%s>[SVA:%d EVA:%d] <-> ", sub_current->is_hole ? "HOLE" : "PROCESS", sub_current->SVA, sub_current->EVA);
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
