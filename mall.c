#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "mall.h"

#define PAGEALIGN(real) (real % PAGESZ) ? real / PAGESZ + 1 : real / PAGESZ
#define WORDALIGN(real) ((real + WORDSZ - 1) & ~(WORDSZ - 1))

void* mmalloc(const size_t bytes) {

    size_t real = WORDALIGN(bytes);

    NODEPTR chunk = __getFree(real);

    if (chunk == NULL) {

        if (heap_start == NULL)
            heap_start = sbrk(0);

        // Max size of allocatable block is 255 * 4096 bytes = 1020 KB
        // Change to u_int16_t for a maximum of 255 MB
        u_int8_t pginc = PAGEALIGN(real);

        pages += pginc;

        void* ptr;

        if ((ptr = sbrk(PAGESZ * pginc)) == (void*)(-1)) {
            fprintf(stderr, "[-] Couldn't allocate memory.\n");
            return NULL;
        }

        for (int i = 0; i < PAGESZ*pginc; i++) {
            ((char *)(ptr))[i] = '\x00';
        }

        HEADERPTR chunk = (HEADERPTR)ptr;
        chunk->magic = NONFREE;

        if (((PAGESZ * pginc) - (real + HEADSZ)) < 24) {
            chunk->size = (PAGESZ * pginc);

        } else {
            chunk->size = real;

            NODEPTR remain = ((void *)chunk)+chunk->size+HEADSZ;
            remain->size = (PAGESZ * pginc) - (real + HEADSZ + HEADSZ);
            remain->magic = FREE;

            addNodeToFreeList(remain);
        }

        return (void *)chunk + HEADSZ;
    }

    size_t rsz = chunk->size;

    // create a header object and return that
    HEADERPTR retptr = ((HEADERPTR)chunk);
    retptr->magic = NONFREE;
    retptr->size  = real;

    // if remainder is less than 24...
    if (rsz - (real + HEADSZ) < 24) {
        retptr->size = rsz;

        removeNodeFromFreeList(chunk);

    } else {
        NODEPTR remain = (void*)retptr + retptr->size + HEADSZ;
        remain->size = rsz - (real + HEADSZ);
        remain->magic = FREE;

        addNodeToFreeList(remain);
    }

    return (void*)retptr+HEADSZ;
}

void coalesce() {
    if (head == NULL)
        return;

    NODEPTR temp = head;
    NODEPTR pre = NULL;

    while (temp != NULL) {

        if (pre != NULL) {

            if (((void *)pre + pre->size + HEADSZ) == temp) {

                NODEPTR remain = (void*)pre;
                remain->size = pre->size + temp->size + HEADSZ;
                remain->magic = FREE;

                pre = remain;
                remain->next = temp->next;

                temp->magic = 0;
                temp->size = 0;
                temp->next = NULL;

                temp = remain->next;

                if (remain->size + HEADSZ == pages * PAGESZ) {
                    void* ptr;
                    if ((ptr = sbrk((PAGESZ * pages)*-1)) == (void*)(-1)) {
                        printf("[-] Failed to return memory to OS");
                    }
                    head = NULL;
                    heap_start = NULL;
                    pages = 0;
                }
                continue;
            }
        }

        pre = temp;
        temp = temp->next;
    }
}

void addNodeToFreeList(NODEPTR nd) {
    if (head == NULL)
        head = nd;
        nd->next = NULL;

    NODEPTR temp = head;
    NODEPTR pre = NULL;

    while (temp != NULL) {

        if (pre == NULL && temp > nd) {
            nd->next = head;
            head=nd;
            pre = nd;
        } else if (pre < nd && temp > nd) {
            pre->next = nd;
            nd->next = temp;
        } else if (temp < nd && temp->next == NULL) {
            temp->next = nd;
            nd->next = NULL;
        }

        pre = temp;
        temp = temp->next;
    }
}

void removeNodeFromFreeList(NODEPTR nd) {
    if (head == NULL)
        return;

    NODEPTR temp = head;
    NODEPTR pre = NULL;

    while (temp != NULL) {

        if (temp == nd) {
            if (pre != NULL) {
                pre->next = temp->next;
            } else {
                head = NULL;
            }
            break;
        }
        pre = temp;
        temp = temp->next;
    }
}

void mfree(const void* ptr) {
    if (ptr == NULL)
        return;

    NODEPTR pointer = (void *)ptr - HEADSZ;

    if (pointer->magic == FREE) {
        printf("[-] Attempt free on already free block.\n");
        return;
    }

    pointer->size  = pointer->size;
    pointer->magic = FREE;

    addNodeToFreeList(pointer);
    coalesce();
}

NODEPTR __getFree(size_t bytes) {
    if (head == NULL)
        return NULL;

    NODEPTR temp = head;
    NODEPTR pre = NULL;

    while (temp != NULL) {

        if (temp->size + HEADSZ >= bytes) {
            removeNodeFromFreeList(temp);
            break;
        }
        pre = temp;
        temp = temp->next;
    }
    return temp;
}

void mfreewalk() {
    NODEPTR temp = head;

    while (temp != NULL) {
        printf("{ %p [%ld] }-->", temp, temp->size + HEADSZ);
        temp = temp->next;
    }
    printf("\n");
}

void mheap() {
    if (pages == 0) {
        printf("No pages allocated.\n");
        return;
    }

    HEADERPTR temp = (HEADERPTR)heap_start;

    printf("--{ mem audit | lil endian }----------\n");

    while (temp->size > 0) {
        printf("  Beginning: %p\n", temp + 1);
        printf("  Pointer:  %s%p%s\n", Bo, temp, Bo);
        printf("     Size:  %s%ld%s\n", Bo, temp->size, Bo);
        printf("     TSize: %s%ld%s\n", Bo, temp->size+HEADSZ, Bo);

        char* magic = temp->magic == FREE ? B : M;

        printf("     Magic: %s%08x%s\n", magic, temp->magic, E);
        printf("     Value:");

        for (int i = 0; i < temp->size + HEADSZ; ++i) {
            if (i % 32 == 0)
                printf("\n");

            unsigned int hx = ((u_int8_t *)(temp))[i];
            hx != 0 ? printf("%s  %02x", P, hx) : printf("%s  %02x", E, hx);
        }
        printf("\n\n\n");

        temp = (void *)temp + temp->size + HEADSZ;
        if (((void *)temp - heap_start) / 4096 == pages) {
            break;
        }

    }
    printf("%s-------------------------------------\n\n", E);
}
