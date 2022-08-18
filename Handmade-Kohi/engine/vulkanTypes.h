#pragma once
#include "../defines.h"


#include "asserts.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(expr)    \
{                           \
    KASSERT(expr == VK_SUCCESS)  \
}                                   \

struct vulkan_buffer{

    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;

};

struct vulkanSwapchainSupportInfo {
      VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
};

#define OBJECT_SHADER_STAGE_COUNT 2

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

    VkCommandPool graphicsCommandPool;

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
  READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
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
    VkImageView* attachments;
    vulkanRenderpass* renderPass;

};

struct vulkanSwapchain{

      VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR swapHandle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;

    vulkanImage depthAttachment;

    // framebuffers used for on-screen rendering.
    vulkanFrameBuffer* frameBuffers;

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
    bool isSignaled;

}vulkanFence;

struct vulkan_pipeline{

VkPipeline handle;
VkPipelineLayout pipeline_layout;

};

typedef struct vulkan_shader_stage{

    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info; 

}vulkan_shader_stage;

typedef struct vulkan_object_shader{

vulkan_shader_stage stages[2];

VkDescriptorPool global_descriptor_pool;
VkDescriptorSetLayout global_descriptor_set_layout;

VkDescriptorSet global_descriptor_sets[3];

global_uniform_object global_ubo;

vulkan_buffer global_uniform_buffer;

vulkan_pipeline pipeline;

}vulkan_object_shader;

typedef struct vulkanContext{

    // The framebuffer's current width.
    u32 framebufferWidth;

    // The framebuffer's current height.
    u32 framebufferHeight;

    // Current generation of framebuffer size. If it does not match framebuffer_size_last_generation,
    // a new one should be generated.
    u64 framebufferSizeGeneration;

    // The generation of the framebuffer when it was last created. Set to framebuffer_size_generation
    // when updated.
    u64 framebufferSizeLastGeneration;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR contextSurface;


    VkDebugUtilsMessengerEXT debugMessenger;


    vulkanDevice device;

    vulkanSwapchain swapchain;
    vulkanRenderpass mainRenderPass;

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;

    // darray
    vulkanCommandBuffer* graphicsCommandBuffers;

    // darray
    VkSemaphore* imageAvailableSemaphores;

    // darray
    VkSemaphore* queueCompleteSemaphores;

    u32 inFlightFenceCount;
    vulkanFence* inFlightFences;

    // Holds pointers to fences which exist and are owned elsewhere.
    vulkanFence** imagesInFlight;

    u32 imageIndex;
    u32 currentFrame;

    b8 recreatingSwapchain;

    vulkan_object_shader object_shader;
    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

    i32 (*findMemoryIndex)(u32 type_filter, u32 property_flags);

  
}vulkanContext;



