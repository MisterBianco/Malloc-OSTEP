#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef MALLOC_H_
#define MALLOC_H_

#define E  "\x1B[0m"
#define Bo "\x1B[1m"
#define F  "\x1B[31m"
#define P  "\x1B[32m"
#define W  "\x1B[33m"
#define B  "\x1B[34m"
#define M  "\x1B[35m"
#define C  "\x1B[36m"
#define Wh "\x1B[37m"

#define HEADSZ sizeof(struct __header_t)
#define PAGESZ getpagesize()
#define WORDSZ 8

#define FREE    0xbaaaaaad
#define NONFREE 0xdeaddead

typedef struct __node_t* NODEPTR;
typedef struct __header_t* HEADERPTR;

struct __node_t {
    size_t size;
    unsigned int magic;
    struct __node_t* next;
};

struct __header_t {
    size_t size;
    unsigned int magic;
};

static NODEPTR head = NULL;

static void* heap_start = NULL;
static unsigned int pages = 0;

void* mmalloc(const size_t bytes);
void mfree(const void* ptr);

void addNodeToFreeList(struct __node_t* nd);
void removeNodeFromFreeList(struct __node_t* nd);

void mheap();
void mfreewalk();

NODEPTR __getFree(size_t bytes);

#endif
