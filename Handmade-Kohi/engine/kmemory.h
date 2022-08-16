#pragma once
#include "platform.h"
#include "kstring.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "memoryTypes.h"

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
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

static memory_system_state * memstate_ptr;

KINLINE void initializeMemory(u64 * memory_requirements, void * state){
    *memory_requirements = sizeof(memory_system_state);
    if(state== 0){

        return;
    };
    memstate_ptr = (memory_system_state*)state;
    memstate_ptr->alloc_count = 0;

platformZeroMemory(&memstate_ptr->stats, sizeof(memstate_ptr->stats));}

KINLINE void shutDownMemory(){
state_ptr = 0;

}
static void * kallocate(u64 size, memory_tag tag){

    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }

    if(state_ptr){


    
    memstate_ptr->stats.totalAllocated += size;
    memstate_ptr->stats.taggedAllocations[tag] += size;
    memstate_ptr->alloc_count++;
    }


    void * block = platformAllocate(size, false);
    platformZeroMemory(block, size);
    return block;
}

KINLINE KAPI void kfree(void* block, u64 size, memory_tag tag){

if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }
     memstate_ptr->stats.totalAllocated -= size;
    memstate_ptr->stats.taggedAllocations[tag] -= size;
    platformFree(block, false);
}

KAPI void* kzeroMemory(void* block, u64 size){
    return platformZeroMemory(block, size);
}

 static void* kcopyMemory(void* dest, const void* source, u64 size){
    return platformCopyMemory(dest, source, size);
}

KINLINE KAPI void* ksetMemory(void* dest, i32 value, u64 size){

    return platformSetMemory(dest, value, size);
}

static char* getMemoryUsageStr(){

     const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (memstate_ptr->stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = memstate_ptr->stats.taggedAllocations[i] / (float)gib;
        } else if (memstate_ptr->stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = memstate_ptr->stats.taggedAllocations[i] / (float)mib;
        } else if (memstate_ptr->stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = memstate_ptr->stats.taggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)memstate_ptr->stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* out_string = stringDuplicate(buffer);
    return out_string;
}

u64 get_memory_alloc_count(){

    if(memstate_ptr){

        return memstate_ptr->alloc_count;
    }
    return 0;
}