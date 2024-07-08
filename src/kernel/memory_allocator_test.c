#include "memory/mod.h"

void testBuddyAllocator() {
    char* p1 = (char*)Malloc(100);
    char* p2 = (char*)Malloc(100);
    *p1 = 'a';
    *(p1+1) = 'b';
    *(p1+2) = 'c';
    *(p1+3) = '\0';

    *p2 = 'd';
    *(p2+1) = 'e';
    *(p2+2) = 'f';
    *(p2+3) = 'g';
    *(p2+4) = '\0';

    Printf("p1=%s, p2=%s\n", p1, p2);

    Free(p1);

    char* p3 = (char*)Malloc(100);
    *p3 = 'h';
    *(p3+1) = 'i';
    *(p3+2) = 'j';
    *(p3+3) = '\0';

    Printf("p3=%s\n", p3);

    Free(p3);
    Free(p2);
}

void testStableAllocator() {
    char* p1 = (char*)StableMalloc(100);
    char* p2 = (char*)StableMalloc(100);
    *p1 = 'a';
    *(p1+1) = 'b';
    *(p1+2) = 'c';
    *(p1+3) = '\0';

    *p2 = 'd';
    *(p2+1) = 'e';
    *(p2+2) = 'f';
    *(p2+3) = 'g';
    *(p2+4) = '\0';

    Printf("p1=%s, p2=%s\n", p1, p2);

    StableFree(p1);

    char* p3 = (char*)StableMalloc(100);
    *p3 = 'h';
    *(p3+1) = 'i';
    *(p3+2) = 'j';
    *(p3+3) = '\0';

    Printf("p3=%s\n", p3);

    StableFree(p3);
    StableFree(p2);

    Printf("StableAllocator test passed\n");
}