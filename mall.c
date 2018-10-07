#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "mall.h"

/*
 [ ] heap grows
 [ ] free blocks are resused
 [ ] free blocks are split and combined
 [ ] free blocks are linked list in sorted order
 [ ] heap is a heap...
 */

void* mmalloc(const size_t bytes) {

    size_t real = ((bytes + WORDSZ - 1) & ~(WORDSZ - 1));

    struct __node_t* chunk = __find_free(real);

    if (chunk == NULL) {
        if (heap_start == NULL)
            heap_start = sbrk(0);

        void* ptr;
        if ((ptr = sbrk(PAGESZ)) == (void*)(-1)) {
            fprintf(stderr, "[-] Couldn't allocate memory.\n");
            return NULL;
        }

        struct __header_t* chunk = (struct __header_t*)ptr;

        chunk->magic = NONFREE;

        if ((PAGESZ - (real + HSZ)) < 24) {
            chunk->size = PAGESZ;

        } else {

            chunk->size = real;

            struct __node_t* remain = ((void *)chunk)+chunk->size+HSZ;
            remain->size = PAGESZ - (real + HSZ + HSZ);
            remain->magic = FREE;

            printf("Pointer: %p\n", remain);
            printf("Size:    %ld\n", remain->size);
            printf("RSIZE:   %ld\n", PAGESZ - remain->size);

            if (head == NULL) {
                head = remain;

            } else {
                remain->next = head;
                head = remain;
            }
        }

        return (void *)chunk + HSZ;
    }

    size_t rsz = chunk->size;

    // printf("Pointer: %p\n", chunk);
    // printf("Size:    %ld\n", real + HSZ);

    // create a header object and return that
    struct __header_t* retptr = ((struct __header_t*)chunk);
    retptr->magic = NONFREE;
    retptr->size  = real;

    struct __node_t* remain = (void*)retptr + retptr->size + HSZ;
    remain->size = rsz - (real + HSZ);
    remain->magic = FREE;

    printf("Pointer: %p\n", retptr);
    printf("Size:    %ld\n", retptr->size);

    printf("Pointer: %p\n", remain);
    printf("Size:    %ld\n", remain->size);

    // Add chunk to linked list
    if (head == NULL)
        head = remain;
    else {
        remain->next = head;
        head = remain;
    }

    // printf("nchunk: %p\n", chunk);

    return (void*)retptr+real;

    // return chunk;
}

void mfree(const void* ptr) {
    if (ptr == NULL)
        return;

    struct __header_t* pointer = ((struct __header_t*)ptr) - 1;

    if (pointer->magic == FREE) {
        printf("[-] Attempt free on already free block.\n");
        return;
    }

    // If not already free

    struct __node_t* temp = (struct __node_t*)pointer;

    temp->size  = pointer->size;
    temp->magic = FREE;

    // if free list is empty
    if (head == NULL) {
        head = temp;
        temp->next = NULL;
    } else {
        struct __node_t* htop = head;

        while (htop != NULL) {
            if (htop < temp && htop->next > temp) {

                htop->next = temp;

                printf("[+] Placed %p in list before: %p and after: %p\n", temp, htop, htop->next);
                break;

                // if added to end
            } else if (temp < htop) {
                htop->next = htop;
                htop = temp;

                printf("[+] Placed %p in list before: %p\n", temp, htop);
                break;
            } else if (temp > htop && htop->next == NULL) {
                htop->next = temp;
                break;

            }
            htop = htop->next;
        }
    }
}

struct __node_t* __find_free(size_t bytes) {
    if (head == NULL)
        return NULL;

    struct __node_t* temp = head;
    struct __node_t* pre = NULL;

    while (temp != NULL) {
        if (temp->size + HSZ >= bytes) {
            // remove temp from list
            // FIX ME
            break;
        }
        pre = temp;
        temp = temp->next;
    }
    return temp;
}

void __walk_free() {
    struct __node_t* temp = head;

    while (temp != NULL) {
        printf("-{ %p }-\n", temp);
        printf("  ----> %ld\n", temp->size);
        temp = temp->next;
    }
}

void mem_audit() {
    struct __header_t* temp = (struct __header_t*)heap_start;
    if (temp == NULL)
        return;

    printf("--{ mem audit | lil endian }----------\n");

    while (temp != NULL && temp->size > 0) {
        printf("  Pointer:  %s%p%s\n", Bo, temp, Bo);
        printf("     Size:  %s%ld%s\n", Bo, temp->size, Bo);
        printf("     TSize: %s%ld%s\n", Bo, temp->size+HSZ, Bo);

        char* magic = temp->magic == FREE ? B : M;

        printf("     Magic: %s%08x%s\n", magic, temp->magic, E);
        printf("     Value:");

        for (int i = 0; i < temp->size + HSZ; ++i) {
            if (i % 32 == 0) {
                printf("\n\t");
            }
            unsigned int hx = ((u_int8_t *)(temp))[i];
            if (hx != 0) {
                printf("%s", P);
            } else {
                printf("%s", E);
            }
            // printf(" %d: %p", i, &temp[i]);
            printf("  %02x", hx);
            // printf("%d\n", i);
        }
        printf("\n\n\n");

        // temp = temp + temp->size - HSZ;
        temp = (void *)temp + temp->size + HSZ;

        // Test for memory page boundaries
        // THIS IS A TERRIBLE SOLUTION...
        // I AM SORRY
        if (((uintptr_t)temp) % 4096 == 0) {
            temp = NULL;
        }

        // printf("temp size: %ld\n", temp->size+HSZ);
    }
    printf("-------------------------------------\n\n");
}
