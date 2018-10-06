#include <stdio.h>
#include <unistd.h>

#include "mall.h"

void* mmalloc(const size_t bytes) {

    // printf("[+] Requested size: %ld\n", bytes);
    
    // Get real alloc size
    size_t real = ((bytes + WORDSZ - 1) & ~(WORDSZ - 1));

    // printf("[+] Actual size: %ld\n", real);

    // Check free list for free nodes
    nodeptr chunk = __find_free(real);

    if (chunk == NULL) {

        if (heap_start == NULL)
            heap_start = sbrk(0);

        // Get a header object and return its address
        // It gets a 1024 byte page meaning we need to split it...
        void* ptr;
        if ((ptr = sbrk(1024)) == (void*)(-1)) {
            fprintf(stderr, "[-] Couldn't allocate memory.\n");
            return NULL;
        }

        // if getting the heap pointer succeeds
        struct __header_t* chunk = (struct __header_t*)ptr;

        chunk->size  = real;
        chunk->magic = NONFREE;

        struct __node_t* remain = ((void *)chunk)+chunk->size+HSZ;
        
        remain->size = 1024 - (chunk->size+HSZ);
        remain->magic = FREE;

        if (head == NULL) {
            head = remain;
            remain->prev = NULL;
            remain->next = NULL;
        } else {
            // Add mo code
        }
        
        // printf("   -- %p\n", heap_start);

        // Create a node of (1024 - real) size bytes in size
        // make the node the head (if head == null)
        printf("[-] New Pointer: %p\n", (void*)chunk);
        printf("[+] Left:        %p\n", remain);
        printf("[+] Size:        %ld\n", remain->size);
        return (void *)chunk + HSZ;

    } else {

        // if node exists then check size of node and split if necessary.
        // return the fcking node.

        printf("Node already exists.\n");

    }

    return chunk+real;
}

void mfree(const void* ptr) {
    if (ptr == NULL)
        return;

    headptr pointer;

    pointer = (headptr)(ptr - HSZ);
    // printf("[+] Freeing block: %p\n", pointer);
    
    // Already free
    if (pointer->magic == FREE) {
        printf("[-] Attempt free on already free block.\n");
        return;

    // If not already free
    } else if (pointer->magic == NONFREE) {

        nodeptr temp = (nodeptr)pointer;

        temp->size  = pointer->size;
        temp->magic = FREE;
        
        // If first block freed
        if (head == NULL) {
            head = temp;
            temp->prev = NULL;
            temp->next = NULL;

        // Move temp to start of linked list
        } else {
            temp->prev = NULL;
            temp->next = head;
            head = temp;
        }

        // printf("[+] Freeing block of size: %ld\n", temp->size);
    }
}

nodeptr __find_free(size_t bytes) {
    if (head == NULL) 
        return NULL;

    nodeptr temp = head;

    while (temp != NULL) {
        if (temp->size >= bytes) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void mem_audit() {
    headptr temp = (headptr)heap_start;

    printf("--{ mem audit }----------------------\n");

    while (temp->size != 0) {
        printf("  Pointer:  %p\n", temp);
        printf("     Size:  %ld\n", temp->size);
        printf("     Magic: %08x\n", temp->magic);
        printf("     Value:");
        
        for (int i = 0; i < temp->size + HSZ; ++i) {
            if (i % 15 == 0) {
                printf("\n\t");
            }
            printf("  %02x", ((u_int8_t *)(temp))[i]);
        }
        printf("\n\n\n");

        // temp = temp + temp->size - HSZ;
        temp = (void *)temp + temp->size + HSZ;
        printf("temp size: %p\n", temp);
    }
    printf("-------------------------------------\n\n");
}

/* 
 The walk function should get the top of the heap and get the
 First object as a header and get the size... 
 
 After getting the size it should walk to the end of the size and
 get the next object

 104
 -16

 */