#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef MALLOC_H_
#define MALLOC_H_

#define WORDSZ 8

#define FREE    0xbaaaaaad
#define NONFREE 0xdeaddead

#define HSZ sizeof(struct __header_t)
#define NSZ sizeof(struct __node_t)

typedef struct __node_t* nodeptr; 
typedef struct __header_t* headptr;

struct __node_t {
    size_t size;
    unsigned int magic;

    struct __node_t* next;
    struct __node_t* prev;
    
} node;

// No point in changing the packing of this object no 
// performance or memory gain is achieved.
struct __header_t {

    size_t size;
    unsigned int magic;
    
} header;

static struct __node_t* head = NULL;

static void* heap_start = NULL;

void* mmalloc(const size_t bytes);
void mfree(const void* ptr);

void mem_audit();
void __walk_free();

struct __node_t* __find_free(size_t bytes);

#endif