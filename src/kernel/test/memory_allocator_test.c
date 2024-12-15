#include "mod.h"

/// @brief 测试伙伴分配器
void testBuddyAllocator() {
    PrintWithColor(GREEN, ">>>>>>>>>>> Test op_overload_test Start <<<<<<<<<<<\n");

    char *p1 = (char *)Malloc(100);
    char *p2 = (char *)Malloc(100);
    *p1 = 'a';
    *(p1 + 1) = 'b';
    *(p1 + 2) = 'c';
    *(p1 + 3) = '\0';

    *p2 = 'd';
    *(p2 + 1) = 'e';
    *(p2 + 2) = 'f';
    *(p2 + 3) = 'g';
    *(p2 + 4) = '\0';

    Printf("p1=%s, p2=%s\n", p1, p2);

    Free(p1);

    char *p3 = (char *)Malloc(100);
    *p3 = 'h';
    *(p3 + 1) = 'i';
    *(p3 + 2) = 'j';
    *(p3 + 3) = '\0';

    Printf("p3=%s\n", p3);

    Free(p3);
    Free(p2);

    PrintWithColor(GREEN, ">>>>>>>>>>> Test op_overload_test PASSED <<<<<<<<<<\n");
}