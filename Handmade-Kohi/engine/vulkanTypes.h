#pragma once
#include "../defines.h"
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
    VkCommandPool graphicsCommandPool;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    VkFormat depthFormat;
};

struct vulkanImage{

    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;

};

enum vulkanRenderPassState{

    READY, RECORDING, RECORDING_ENDED, SUBMITTED, NOT_ALLOCATED
};

struct vulkanRenderpass{

VkRenderPass handle;
f32 x, y, w, h;
f32 r, g, b, a;
f32 depth;
u32 stencil;
vulkanRenderPassState state;

};

struct vulkanFrameBuffer{

    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView * attachments;
    vulkanRenderpass * renderPass;

};

struct vulkanSwapchain{

    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR swapHandle;
    u32 imageCount;
    VkImage * images;
    VkImageView * views;
    vulkanImage depthAttachment;
    vulkanFrameBuffer * frameBuffers;

};

enum vulkanCommandBufferState{

    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
};

 struct vulkanCommandBuffer{

    VkCommandBuffer handle;
    vulkanCommandBufferState state;

 };

typedef struct vulkanFence{

    VkFence handle;
    b8 isSignaled;

}vulkanFence;

struct vulkanContext{

    u32 framebufferWidth;
    u32 framebufferHeight;

    u64 framebufferSizeGeneration;
    u64 framebufferSizeLastGeneration;

    VkInstance instance;
    VkAllocationCallbacks * allocator;
    VkSurfaceKHR contextSurface;
    #if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
    #endif
    vulkanDevice device;
    vulkanCommandBuffer * graphicsCommandBuffers;
    VkSemaphore * imageAvailableSemaphores;
    VkSemaphore * queueCompleteSemaphores;
    u32 inFlightFenceCount;
    vulkanFence * inFlightFences;
    vulkanFence ** imagesInFlight;
    vulkanSwapchain swapchain;
    vulkanCommandBuffer * commandBuffer;
    u32 imageIndex;
    u32 currentFrame;
    bool recreatingSwapchain;
    i32(*findMemoryIndex)(u32 typeFilter, u32 propertyFlags); 
    vulkanRenderpass mainRenderPass;
  
};

