#include <stdio.h>
#include <stdio.h>
#include <string.h>

#include "mall.h"

int main() {
    printf("--{ Hello }------------------\n");

    int* p = mmalloc(sizeof(int));
    // int* q = mmalloc(sizeof(int));
    // int* r = mmalloc(sizeof(int));
    // int* s = mmalloc(sizeof(int));
    // int* t = mmalloc(sizeof(int));
    // int* u = mmalloc(sizeof(int));
    // int* v = mmalloc(sizeof(int));

    *p = 5;
    // *q = 15;

    mfree(p);
    // mfree(q);
    // mfree(r);
    // mfree(s);
    // mfree(t);
    // mfree(u);
    // mfree(v);

    mem_audit();

    printf("----------------{ Goodbye }--\n");

    return 0;
}

// https://github.com/FiethPaste/OSTEP-Project3-DynamicMemoryAllocation/blob/master/mem.c
