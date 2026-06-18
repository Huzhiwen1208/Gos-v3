#pragma once
#include "../common/type.h"
#include "type.h"

void InitializeGDT();
void SetTSSEsp0(u32 esp0);