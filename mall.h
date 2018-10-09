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
#define Wh  "\x1B[37m"

#define WORDSZ 8
#define PAGESZ getpagesize()

#define FREE    0xbaaaaaad
#define NONFREE 0xdeaddead

#define HSZ sizeof(struct __header_t)
#define NSZ sizeof(struct __node_t)

struct __node_t {
    size_t size;
    unsigned int magic;

    struct __node_t* next;

} node;

struct __header_t {

    size_t size;
    unsigned int magic;

} header;

static struct __node_t* head = NULL;

static void* heap_start = NULL;
static unsigned int pages = 0;

void* mmalloc(const size_t bytes);
void mfree(const void* ptr);

void add_node(struct __node_t* nd);
void remove_node(struct __node_t* nd);

void mem_audit();
void __walk_free();

struct __node_t* __find_free(size_t bytes);

#endif
