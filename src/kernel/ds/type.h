#pragma once

#include "../common/type.h"

#define HASH_TABLE_SIZE 101

/// @brief 哈希表中的键值对结点
// hash
typedef struct Value {
    void *key;
    void *value;
    Boolean Available;
} Value;

/// @brief 哈希表
typedef struct HashTable {
    Value Table[HASH_TABLE_SIZE];
    char *KeyType;
    char *ValueType;
    Boolean Initialized;

    void *(*Get)(struct HashTable *self, void *key);
    void (*Put)(struct HashTable *self, void *key, void *value);
    void (*Delete)(struct HashTable *self, void *key);
} HashTable;