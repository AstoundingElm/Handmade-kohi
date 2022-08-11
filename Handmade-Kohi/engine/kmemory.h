#pragma once
#include "/home/petermiller/Desktop/Handmade-Kohi/engine/platform.h"
#include "/home/petermiller/Desktop/Handmade-Kohi/engine/kstring.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "/home/petermiller/Desktop/Handmade-Kohi/engine/memoryTypes.h"

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
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

memoryStats stats;

static inline void initializeMemory(){
platformZeroMemory(&stats, sizeof(stats));}

void shutDownMemory(){


}
KAPI void * kallocate(u64 size, memory_tag tag){

    if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }
    stats.totalAllocated += size;
    stats.taggedAllocations[tag] += size;

    void * block = platformAllocate(size, false);
    platformZeroMemory(block, size);
    return block;
}

static inline KAPI void kfree(void* block, u64 size, memory_tag tag){

if(tag == MEMORY_TAG_UNKNOWN){
        KWARN("Kallocat called using MEMORY_TAG_UNKNOWN. Re-class this allocation!");
    }
     stats.totalAllocated -= size;
    stats.taggedAllocations[tag] -= size;
    platformFree(block, false);
}

static inline KAPI void* kzeroMemory(void* block, u64 size){
    return platformZeroMemory(block, size);
}

 KAPI void* kcopyMemory(void* dest, const void* source, u64 size){
    return platformCopyMemory(dest, source, size);
}

static inline KAPI void* ksetMemory(void* dest, i32 value, u64 size){

    return platformSetMemory(dest, value, size);
}

char* getMemoryUsageStr(){

     const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.taggedAllocations[i] / (float)gib;
        } else if (stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.taggedAllocations[i] / (float)mib;
        } else if (stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.taggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* out_string = stringDuplicate(buffer);
    return out_string;
}
