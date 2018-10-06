#include <stdio.h>
#include <stdio.h>

#include "mall.h"

int main() {
    printf("--{ Hello }------------------\n");

    int* p = mmalloc(sizeof(int));
    int* q = mmalloc(sizeof(int));

    printf("p : %p\n", p);
    printf("q : %p\n", q); // should end in 30
    // printf("r : %p\n", r);

    mfree(p);
    // mfree(q);
    // mfree(r);

    // mem_audit();

    printf("----------------{ Goodbye }--\n");

    return 0;
}

// https://github.com/FiethPaste/OSTEP-Project3-DynamicMemoryAllocation/blob/master/mem.c
