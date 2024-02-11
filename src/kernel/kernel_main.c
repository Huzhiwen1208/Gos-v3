#include "common/mod.h"
#include "console/mod.h"

void KernelMain() {
    String message = "Hello kernel output";
    ConsoleWriteWithColor(message, StringLength(message), BLUE);
}