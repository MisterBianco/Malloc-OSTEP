#include <stdio.h>
#include <stdio.h>

#include "mall.h"

int main() {
    printf("--{ Hello }------------------\n");
    int* p = mmalloc(200);

    *p = 83593845;

    int* r = mmalloc(sizeof(int));

    *r = 16;

    mem_audit();

    mfree(p);
    mfree(r);

    mem_audit();

    return 0;
}

// https://github.com/FiethPaste/OSTEP-Project3-DynamicMemoryAllocation/blob/master/mem.c