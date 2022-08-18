#pragma once

#include "../defines.h"
#include "kmemory.h"
#include "string.h"
//#include "memoryTypes.h"

#include <stdarg.h>
#include <stdio.h>

KINLINE void* kcopyMemory(void* dest, const void* source, u64 size);
KINLINE void * kallocate(u64 size, memory_tag tag);

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

 template<typename T> KINLINE i32 string_format_v( char* dest, const char* format, T * va_listp) {
    if (dest) {
        // Big, but can fit on the stack.
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format,va_listp);
        buffer[written] = 0;
        kcopyMemory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}


KINLINE i32 string_format(char* dest, const char* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

