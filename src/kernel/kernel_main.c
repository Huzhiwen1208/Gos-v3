#include "common/mod.h"
#include "console/type.h"
#include "console/method.h"

void KernelMain() {
    String message = "Hello kernel output";
    ConsoleWriteWithColor(message, StringLength(message), RED);
}