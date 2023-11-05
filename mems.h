#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

typedef struct Mapping {
    void* SPA;
    void* EPA;
    int SVA;
    int EVA;
} mapp_t;

typedef struct sub_t {
    mapp_t mapping;  // Include mapp_t structure in sub_t
    void* mem_start_addr;
    size_t mem_size;
    void* mem_end_addr;
    int is_hole;
    struct sub_t* next;
    struct sub_t* prev;
} sub_t;

typedef struct main_t {
    mapp_t mapping;
    void* mem_start_addr;
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
        newnode->mapping.SVA = starting;
        newnode->mapping.EVA = starting + allocate - 1;

        newnode->mem_start_addr = newnode->mapping.SVA;
        newnode->current = newnode->mem_start_addr;
        newnode->mem_size = allocate;
        newnode->remaining = allocate;
        newnode->prev = NULL;
        newnode->next = NULL;

        // Initialize the process sub-node within the main node
        sub_t* sub_chain = &(newnode->sub_chain); // Create a temporary pointer
        sub_chain->mapping.SPA = newnode->mapping.SPA;
        sub_chain->mapping.EPA = newnode->mapping.SPA + size - 1;
        sub_chain->mapping.SVA = newnode->mapping.SVA;
        sub_chain->mapping.EVA = newnode->mapping.SVA + size - 1;
        sub_chain->mem_start_addr = newnode->mem_start_addr;
        sub_chain->mem_end_addr = sub_chain->mem_start_addr + size;
        sub_chain->mem_size = size;
        sub_chain->is_hole = 0;  // The current sub-node is not a hole, it's occupied by a process
        sub_chain->next = NULL;  // Initialize the sub-chain's 'next' pointer to NULL

        if (size < allocate) {
            // Create a hole sub-node within the main node for the remaining space
            sub_t* hole_node = (sub_t*)((char*)sub_chain->mem_start_addr + size);
            hole_node->mapping.SPA = sub_chain->mapping.EPA + 1;
            hole_node->mapping.EPA = newnode->mapping.EPA;
            hole_node->mapping.SVA = sub_chain->mapping.EVA + 1;
            hole_node->mapping.EVA = newnode->mapping.EVA;
            hole_node->mem_start_addr = sub_chain->mem_start_addr + size;
            hole_node->mem_end_addr = sub_chain->mem_end_addr;
            hole_node->mem_size = allocate - size;
            hole_node->is_hole = 1;  // The remaining space is a hole
            hole_node->prev = sub_chain;  // Set the 'prev' pointer to the process sub-node
            hole_node->next = NULL;  // There's no sub-node after the hole
        }

        // Update the remaining and current pointers.
        newnode->remaining -= allocate;
        newnode->current += allocate;

        return newnode->mapping.SVA;
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
        printf("MAIN[%d:%d] -> ", current->mapping.SVA, current->mapping.EVA);

        sub_t* sub_current = &current->sub_chain;
        int sub_chain_length = 0;

        while (sub_current != NULL) {
            printf("<%s>[SVA:%d EVA:%d]", sub_current->is_hole ? "HOLE" : "PROCESS", sub_current->mapping.SVA, sub_current->mapping.EVA);
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

