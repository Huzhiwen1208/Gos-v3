#include "stdlib.h"

void syscall_read_test() {
    while (1) {
        char message[1024];
        Read(message, 25);
        printf("Get message: %s\n", message);
    }
}