#pragma once

#include "../common/type.h"

#define HASH_TABLE_SIZE 101

// hash
typedef struct Value {
    void* key;
    void* value;
    Boolean Available;
} Value;

typedef struct HashTable {
    Value Table[HASH_TABLE_SIZE];
    char* KeyType;
    char* ValueType;
    Boolean Initialized;

    void* (*Get)(struct HashTable* self, ...);
    void (*Put)(struct HashTable* self, ...);
    void (*Delete)(struct HashTable* self, ...);
} HashTable;
