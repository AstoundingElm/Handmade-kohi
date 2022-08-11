#pragma once
//#include "vulkanTypes.h"
//#include "logger.h"
//#include "vulkanDevice.h"
//#include "kmemory.h"
#include "vulkanImage.h"

static inline void create(vulkanContext * context, u32 width, u32 height, 
vulkanSwapchain * swapchain ){

    VkExtent2D swapchainExtent = {width, height};
    swapchain->maxFramesInFlight = 2;

     b8 found = false;
    for (u32 i = 0; i < context->device.swapchainSupport.formatCount; ++i) {
        VkSurfaceFormatKHR format = context->device.swapchainSupport.formats[i];
        // Preferred formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->imageFormat = format;
            found = true;
            break;
        }
    }

    if (!found) {
        swapchain->imageFormat = context->device.swapchainSupport.formats[0];
    }


    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < context->device.swapchainSupport.presentModeCount; ++i) {
        VkPresentModeKHR mode = context->device.swapchainSupport.presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
    }

    // Requery swapchain support.
    vulkanDeviceQuerySwapchainSupport(
        context->device.physicalDevice,
        context->contextSurface,
        &context->device.swapchainSupport);

    // Swapchain extent
    if (context->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = context->device.swapchainSupport.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU.
    VkExtent2D min = context->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = KCLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = KCLAMP(swapchainExtent.height, min.height, max.height);

    u32 image_count = context->device.swapchainSupport.capabilities.minImageCount + 1;
    if (context->device.swapchainSupport.capabilities.maxImageCount > 0 && image_count > context->device.swapchainSupport.capabilities.maxImageCount) {
        image_count = context->device.swapchainSupport.capabilities.maxImageCount;
    }

    // Swapchain create info
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface = context->contextSurface;
    swapchainCreateInfo.minImageCount = image_count;
    swapchainCreateInfo.imageFormat = swapchain->imageFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchain->imageFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (context->device.graphicsQueueIndex != context->device.presentQueueIndex) {
        u32 queueFamilyIndices[] = {
            (u32)context->device.graphicsQueueIndex,
            (u32)context->device.presentQueueIndex};
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = 0;
    }

    swapchainCreateInfo.preTransform = context->device.swapchainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = present_mode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(context->device.logicalDevice, &swapchainCreateInfo, context->allocator, &swapchain->swapHandle));

    // Start with a zero frame index.
    context->currentFrame = 0;

    // Images
    swapchain->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->swapHandle, &swapchain->imageCount, 0));
    if (!swapchain->images) {
        swapchain->images = (VkImage*)kallocate(sizeof(VkImage) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    if (!swapchain->views) {
        swapchain->views = (VkImageView*)kallocate(sizeof(VkImageView) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->swapHandle, &swapchain->imageCount, swapchain->images));

    // Views
    for (u32 i = 0; i < swapchain->imageCount; ++i) {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = swapchain->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewInfo, context->allocator, &swapchain->views[i]));
    }

    // Depth resources
    if (!vulkanDeviceDetectDepthFormat(&context->device)) {
        context->device.depthFormat = VK_FORMAT_UNDEFINED;
        KFATAL("Failed to find a supported format! Update your drivers!");
    }

    // Create depth image and its view.
    vulkanImageCreate(
        context,
        VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        context->device.depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &swapchain->depthAttachment);

    KINFO("Swapchain created successfully.");

};

static inline void destroy(vulkanContext * context, vulkanSwapchain * swapchain){

     vulkanImageDestroy(context, &swapchain->depthAttachment);

     for(u32 i = 0; i < swapchain->imageCount; ++i){

        vkDestroyImageView(context->device.logicalDevice, swapchain->views[i], context->allocator);

     };

     vkDestroySwapchainKHR(context->device.logicalDevice, swapchain->swapHandle, context->allocator);

}

static inline void vulkanSwapchainCreate(vulkanContext * context, u32 width, u32 height, 
vulkanSwapchain * outSwapchain){

    create(context, width, height, outSwapchain);


};

static inline void vulkanSwapchainRecreate(vulkanContext * context, u32 width, u32 height,
vulkanSwapchain * swapchain){

    destroy(context, swapchain);
    create(context, width, height, swapchain);


}

static inline void vulkanSwapchainDestroy(vulkanContext * context, vulkanSwapchain * swapchain){

    destroy(context, swapchain);

}

static inline b8 vulkanSwapchainAquireNextImageIndex(vulkanContext * context, vulkanSwapchain * swapchain, u64 timeoutNs, 
VkSemaphore imageAvailableSemaphores, VkFence fence, u32 * outImageIndex){

    VkResult result = vkAcquireNextImageKHR(context->device.logicalDevice, swapchain->swapHandle, timeoutNs, 
    imageAvailableSemaphores,
    fence, outImageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR){

        vulkanSwapchainRecreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
        return false;
    }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){

            KFATAL("Failed to aquire swapchain image");
            return false;
        }
    
    return true;
}

static inline void vulkanSwapchainPresent(vulkanContext * context, vulkanSwapchain * swapchain, 
VkQueue graphicsQueue, VkQueue presentQueue, VkSemaphore renderCompleteSemaphore, u32 presentImageIndex){

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapHandle;
    presentInfo.pImageIndices = &presentImageIndex;
    presentInfo.pResults = 0;

     VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date, suboptimal or a framebuffer resize has occurred. Trigger swapchain recreation.
        vulkanSwapchainRecreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
    } else if (result != VK_SUCCESS) {
        KFATAL("Failed to present swap chain image!");
    }

    context->currentFrame = (context->currentFrame +1) %swapchain->maxFramesInFlight;

}