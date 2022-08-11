#pragma once

#include "vulkanTypes.h"
#include "logger.h"
static inline void vulkanFenceCreate(vulkanContext * context, b8 createSignaled, vulkanFence * outFence){

    outFence->isSignaled  = createSignaled;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(outFence->isSignaled){
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;    
    }

    VK_CHECK(vkCreateFence(context->device.logicalDevice,&fenceCreateInfo, context->allocator, &outFence->handle));
}

static inline void vulkanFenceDestroy(vulkanContext * context, vulkanFence * fence){

    if(fence->handle){
        vkDestroyFence(context->device.logicalDevice, fence->handle, context->allocator);
        fence->handle = 0;
    }
    fence->isSignaled = false;

}

static inline b8 vulkanFenceWait(vulkanContext * context, vulkanFence * fence, u64 timeoutNs){

    if(!fence->isSignaled){

        VkResult result = vkWaitForFences(context->device.logicalDevice, 
        1, &fence->handle, true, timeoutNs);
         switch (result) {
            case VK_SUCCESS:
                fence->isSignaled = true;
                return true;
            case VK_TIMEOUT:
                KWARN("vk_fence_wait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                KERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                KERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                KERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                break;
            default:
                KERROR("vk_fence_wait - An unknown error has occurred.");
                break;
        }
    } else {
        // If already signaled, do not wait.
        return true;
    }

    return false;

}

static inline void vulkanFenceReset(vulkanContext * context, vulkanFence * fence){

    if(fence->isSignaled){

        VK_CHECK(vkResetFences(context->device.logicalDevice, 1, &fence->handle));
        fence->isSignaled = false;
    }

}