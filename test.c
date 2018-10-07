#include <stdio.h>
#include <stdio.h>
#include <string.h>

#include "mall.h"

int main() {
    printf("--{ Hello }------------------\n");

    // int* p = mmalloc(sizeof(int));
    // printf("p : %p\n\n\n", p);
    //
    // int* q = mmalloc(sizeof(int));
    // printf("q : %p\n\n\n", q);
    //
    // int* r = mmalloc(sizeof(int));
    // printf("r : %p\n\n\n", r);
    // int* s = mmalloc(sizeof(int));
    // int* t = mmalloc(sizeof(int));
    // int* u = mmalloc(800);
    //
    // int* v = mmalloc(200);
    // printf("v : %p\n\n\n", v);

    int* w = mmalloc(sizeof(int));
    printf("w : %p\n\n\n", w);

     // should end in 30

    // printf("r : %p\n", r);

    // mfree(p);
    // mfree(q);
    // mfree(r);

    mem_audit();

    printf("----------------{ Goodbye }--\n");

    return 0;
}

// https://github.com/FiethPaste/OSTEP-Project3-DynamicMemoryAllocation/blob/master/mem.c
