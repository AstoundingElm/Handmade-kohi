#pragma once
#include "platform.h"
#include "kstring.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include "memoryTypes.h"

global_variable const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "};

struct memoryStats{

    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};


struct memory_system_state{

    memoryStats stats;
    u64 alloc_count;

};

global_variable memory_system_state * memory_state_ptr;

KINLINE void memory_system_initialize(u64* memory_requirement, void* state) {
   
    *memory_requirement = sizeof(memory_system_state);
    if (state == 0) {
        return;
    }

    memory_state_ptr = (memory_system_state *)state;
    memory_state_ptr->alloc_count = 0;
    platformZeroMemory(&memory_state_ptr->stats, sizeof(memory_state_ptr->stats));

}

KINLINE void memory_system_shutdown(void* state) {
   
    memory_state_ptr = 0;

}

KINLINE void * kallocate(u64 size, memory_tag tag){

    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }

    if(memory_state_ptr){


    
    memory_state_ptr->stats.totalAllocated += size;
    memory_state_ptr->stats.taggedAllocations[tag] += size;
    memory_state_ptr->alloc_count++;
    }


    void * block = platformAllocate(size, false);
    platformZeroMemory(block, size);
    return block;
}

KINLINE KAPI void kfree(void* block, u64 size, memory_tag tag){

if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }
     memory_state_ptr->stats.totalAllocated -= size;
    memory_state_ptr->stats.taggedAllocations[tag] -= size;
    platformFree(block, false);
}

KINLINE KAPI void* kzeroMemory(void* block, u64 size){
    return platformZeroMemory(block, size);
}

 internal void* kcopyMemory(void* dest, const void* source, u64 size){
    return platformCopyMemory(dest, source, size);
}

KINLINE KAPI void* ksetMemory(void* dest, i32 value, u64 size){

    return platformSetMemory(dest, value, size);
}

KINLINE char* getMemoryUsageStr(){

     const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (memory_state_ptr->stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = memory_state_ptr->stats.taggedAllocations[i] / (float)gib;
        } else if (memory_state_ptr->stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = memory_state_ptr->stats.taggedAllocations[i] / (float)mib;
        } else if (memory_state_ptr->stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = memory_state_ptr->stats.taggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)memory_state_ptr->stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* out_string = stringDuplicate(buffer);
    return out_string;
}

KINLINE u64 get_memory_alloc_count(){

    if(memory_state_ptr){

        return memory_state_ptr->alloc_count;
    }
    return 0;
}