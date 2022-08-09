#pragma once
#include "defines.h"
#include "kmemory.h"
#include "string.h"
#include "memoryTypes.h"
void* kcopyMemory(void* dest, const void* source, u64 size);
void * kallocate(u64 size, memory_tag tag);

static inline KAPI u64 stringLength(const char * string){

    return strlen(string);
};
static inline KAPI char * stringDuplicate(const char * string){

    u64 length = stringLength(string);
    char * copy = (char *)kallocate(length + 1, MEMORY_TAG_STRING);
    kcopyMemory(copy, string, length + 1);
    return copy;
};
static inline KAPI b8 stringsEqual(const char * str0, const char * str2){

    return strcmp(str0, str2) == 0;
};