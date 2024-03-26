#include "process/mod.h"

extern void user_process();

void GotoGshell() {
    CreateUserProcess(user_process);
}