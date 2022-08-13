#pragma once
#include "/home/petermiller/Desktop/Handmade-Kohi/defines.h"
#include "kmemory.h"
#include "string.h"
#include "memoryTypes.h"
static void* kcopyMemory(void* dest, const void* source, u64 size);
static void * kallocate(u64 size, memory_tag tag);

KINLINE KAPI u64 stringLength(const char * string){

    return strlen(string);
};
KINLINE KAPI char * stringDuplicate(const char * string){

    u64 length = stringLength(string);
    char * copy = (char *)kallocate(length + 1, MEMORY_TAG_STRING);
    kcopyMemory(copy, string, length + 1);
    return copy;
};
KINLINE KAPI b8 stringsEqual(const char * str0, const char * str2){

    return strcmp(str0, str2) == 0;
};
