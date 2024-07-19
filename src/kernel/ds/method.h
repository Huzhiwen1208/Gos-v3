#pragma once
#include "../common/type.h"
#include "type.h"

HashTable* NewMap(char* keyType, char* valueType);
void DeleteMap(HashTable** map);

Queue* NewQueue(char* type, i32 size);
void DeleteQueue(Queue** self);

ReentrantLock* NewReentrantLock();
