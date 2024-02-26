#include "stdlib.h"

void user_process() {
    while(1) {
        char* message = "Hello, this is user Process\n";
        printf("%s", message);
    }
}