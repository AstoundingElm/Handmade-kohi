#pragma once
#include "../defines.h"
#include "stdio.h"
#include <dlfcn.h>
#include "vulkanFunctions.h"


#define VK_NO_PROTOTYPES

 b8 load_vulkan_functions(){

    void * vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);
    if(vulkan_library == nullptr){

        printf("Failed to open .so");
        return false;
    }

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vulkan_library, "vkGetInstanceProcAddr");
    if(vkGetInstanceProcAddr == nullptr){

        printf("failed to load procaddress");
        return false;
    }

    return true;


};

