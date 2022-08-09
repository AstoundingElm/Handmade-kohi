#pragma once
#include "defines.h"
#include "asserts.cpp"
#include <vulkan/vulkan.h>



#define VK_CHECK(expr)    \
{                           \
    KASSERT(expr == VK_SUCCESS)  \
}                                   \

struct vulkanSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
};

struct vulkanDevice{

    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
      vulkanSwapchainSupportInfo swapchainSupport;
    i32 graphicsQueueIndex;
    i32 presentQueueIndex;
    i32 transferQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
};

struct vulkanContext{

    VkInstance instance;
    VkAllocationCallbacks * allocator;
    VkSurfaceKHR contextSurface;
    #if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
    #endif
    vulkanDevice device;
};

