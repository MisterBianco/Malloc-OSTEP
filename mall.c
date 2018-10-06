#include <stdio.h>
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

    nodeptr chunk = __find_free(real);

    if (chunk == NULL) {

        if (heap_start == NULL)
            heap_start = sbrk(0);

        void* ptr;
        if ((ptr = sbrk(1024)) == (void*)(-1)) {
            fprintf(stderr, "[-] Couldn't allocate memory.\n");
            return NULL;
        }

        struct __header_t* chunk = (struct __header_t*)ptr;

        chunk->magic = NONFREE;

        if ((1024 - (real + HSZ)) < 24) {
            chunk->size = 1024;

        } else {

            chunk->size = real;

            struct __node_t* remain = ((void *)chunk)+chunk->size+HSZ;
            remain->size = 1024 - (real+HSZ);
            remain->magic = FREE;

            if (head == NULL) {
                head = remain;

            } else {
                remain->next = head;
                head = remain;
            }
        }

        return (void *)chunk + HSZ;

    } else if (chunk != NULL) {
        // check to split chunk
            // split
        // return header
    }

    return chunk+real;
}

void mfree(const void* ptr) {
    if (ptr == NULL)
        return;

    headptr pointer = (headptr)(ptr - HSZ);

    if (pointer->magic == FREE) {
        printf("[-] Attempt free on already free block.\n");
        return;

    // If not already free
    } else if (pointer->magic == NONFREE) {

        nodeptr temp = (nodeptr)pointer;

        temp->size  = pointer->size;
        temp->magic = FREE;

        // if free list is empty
        if (head == NULL) {
            head = temp;
            temp->prev = NULL;
            temp->next = NULL;
        } else {
            nodeptr htop = head;
            while (htop != NULL) {

                if (htop < temp && htop->next > temp) {

                    htop->next->prev = temp;
                    htop->next = temp;

                    printf("[+] Placed %p in list before: %p and after: %p\n", temp, htop, htop->next);
                    break;

                    // if added to end
                } else if (temp < htop) {
                    htop->prev = temp;

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
}

nodeptr __find_free(size_t bytes) {
    if (head == NULL)
        return NULL;

    nodeptr temp = head;

    while (temp != NULL) {
        if (temp->size + HSZ >= bytes) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void __walk_free() {
    nodeptr temp = head;

    while (temp != NULL) {
        printf("-{ %p }-\n", temp);
        printf("  ----> %ld\n", temp->size);
        temp = temp->next;
    }
}

void mem_audit() {
    headptr temp = (headptr)heap_start;

    printf("--{ mem audit | lil endian }----------\n");

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
        printf("temp size: %ld\n", temp->size);
    }
    printf("-------------------------------------\n\n");
}
