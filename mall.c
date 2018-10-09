#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "mall.h"

void* mmalloc(const size_t bytes) {

    size_t real = ((bytes + WORDSZ - 1) & ~(WORDSZ - 1));

    struct __node_t* chunk = __find_free(real);

    if (chunk == NULL) {

        if (heap_start == NULL)
            heap_start = sbrk(0);

        u_int8_t pginc = (real % PAGESZ) ? real / PAGESZ + 1 : real / PAGESZ;

        pages += pginc;

        // printf("    [!] New page(s) allocated: %d\n", pginc);
        // printf("    [!] Total Pages: %d\n", pages);

        void* ptr;
        if ((ptr = sbrk(PAGESZ * pginc)) == (void*)(-1)) {
            fprintf(stderr, "[-] Couldn't allocate memory.\n");
            return NULL;
        }

        struct __header_t* chunk = (struct __header_t*)ptr;

        chunk->magic = NONFREE;

        if (((PAGESZ * pginc) - (real + HSZ)) < 24) {
            chunk->size = (PAGESZ * pginc);

        } else {

            chunk->size = real;

            struct __node_t* remain = ((void *)chunk)+chunk->size+HSZ;
            remain->size = (PAGESZ * pginc) - (real + HSZ + HSZ);
            remain->magic = FREE;

            // printf("Pointer: %p\n", remain);
            // printf("Size:    %ld\n", remain->size);

            add_node(remain);
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

    // if remainder is less than 24...
    if (rsz - (real + HSZ) < 24) {
        retptr->size = rsz;

        remove_node(chunk);

    } else {

        // printf("Remainder: %ld\n", rsz);

        struct __node_t* remain = (void*)retptr + retptr->size + HSZ;
        remain->size = rsz - (real + HSZ);
        remain->magic = FREE;

        // printf("retPointer: %p\n", retptr);
        // printf("retSize:    %ld\n", retptr->size);

        // printf("remPointer: %p\n", remain);
        // printf("remSize:    %ld\n", remain->size);

        // Add chunk to linked list
        add_node(remain);

    }

    // printf("nchunk: %ld\n", real);

    return (void*)retptr+HSZ;
}

void coalesce() {
    if (head == NULL)
        return;

    struct __node_t* temp = head;
    struct __node_t* pre = NULL;

    while (temp != NULL) {

        if (pre != NULL) {
            // printf(" ~~ %p : %p : %p\n", pre, (void *)pre + pre->size + HSZ, temp);

            if (((void *)pre + pre->size + HSZ) == temp) {
                // COALESCE!
                // printf("Coalescing: %p : %p\n", pre, temp);

                struct __node_t* remain = (void*)pre;
                remain->size = pre->size + temp->size + HSZ;
                remain->magic = FREE;

                pre = remain;
                remain->next = temp->next;

                // If all memory deallocated
                if (remain->size + HSZ == pages * PAGESZ) {
                    void* ptr;
                    if ((ptr = sbrk((PAGESZ * pages)*-1)) == (void*)(-1)){
                        printf("[-] Failed to return memory to OS");
                    }
                    head = NULL;
                    pages = 0;
                }

            }
        }
        // check if nodes can be joined
        pre = temp;
        temp = temp->next;
    }
}

void add_node(struct __node_t* nd) {
    // printf("Adding node to free list: %p\n", nd);

    if (head == NULL)
        head = nd;
        nd->next = NULL;

    struct __node_t* temp = head;
    struct __node_t* pre = NULL;

    // printf(" > %p\n", nd);

    while (temp != NULL) {
        // if node goes in front
        if (pre == NULL && temp > nd){
            nd->next = head;
            head=nd;
            break;
        } else if (pre < nd && temp > nd) {
            pre->next = nd;
            nd->next = temp;
            break;
        } else if (pre == NULL && nd < temp) {
            head = nd;
            nd->next = temp;
            break;
        } else if (temp < nd && temp->next == NULL) {
            temp->next = nd;
            nd->next = NULL;
            break;
        }
        pre = temp;
        temp = temp->next;
    }
}

void remove_node(struct __node_t* nd) {
    if (head == NULL)
        return;

    struct __node_t* temp = head;
    struct __node_t* pre = NULL;

    while (temp != NULL) {
        // printf("Comparing %p to %p\n", temp, nd);
        // if node goes in the middle
        if (temp == nd) {
            // printf("Equal\n");
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
    // printf("Returning\n");
}

void mfree(const void* ptr) {
    if (ptr == NULL)
        return;

    // printf("Given Freeing: %p\n", ptr);

    struct __node_t* pointer = (void *)ptr - HSZ;

    if (pointer->magic == FREE) {
        printf("[-] Attempt free on already free block.\n");
        return;
    }

    // printf("Found Freeing: %p\n", pointer);

    pointer->size  = pointer->size;
    pointer->magic = FREE;

    // if free list is empty
    add_node(pointer);

    // __walk_free();
    coalesce();
}

struct __node_t* __find_free(size_t bytes) {
    // printf("Requesting %ld Bytes from heap free\n", bytes+HSZ);
    if (head == NULL)
        return NULL;

    struct __node_t* temp = head;
    struct __node_t* pre = NULL;

    while (temp != NULL) {
        // printf("found a chunk with: %ld bytes left\n", temp->size+HSZ);
        if (temp->size + HSZ >= bytes) {
            // printf("Removing node\n");

            remove_node(temp);

            break;
        }
        pre = temp;
        temp = temp->next;
    }
    // printf("Temp: %p\n", temp);
    return temp;
}

void __walk_free() {
    struct __node_t* temp = head;

    while (temp != NULL) {
        printf("{ %p [%ld] }-->", temp, temp->size + HSZ);
        temp = temp->next;
    }
}

void mem_audit() {
    struct __header_t* temp = (struct __header_t*)heap_start;
    if (temp == NULL || pages == 0)
        return;

    printf("--{ mem audit | lil endian }----------\n");

    while (temp->size > 0) {
        printf("  Beginning: %p\n", temp + 1);
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
        if (((void *)temp - heap_start) / 4096 == pages) {
            break;
        }

            // break;


        // printf("temp size: %ld\n", temp->size+HSZ);
    }
    printf("-------------------------------------\n\n");
}
