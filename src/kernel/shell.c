#include "process/method.h"

extern void user_process();

void GotoGshell() {
    CreateUserProcess(user_process);
}