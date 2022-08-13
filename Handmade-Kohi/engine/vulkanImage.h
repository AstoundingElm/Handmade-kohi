#pragma once
#include "vulkanTypes.h"
#include "kmemory.h"
#include "logger.h"

KINLINE void vulkanImageViewCreate(vulkanContext * context, VkFormat format, vulkanImage * image, 
VkImageAspectFlags aspectFlags){

 VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.image = image->handle;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO: Make configurable.
    view_create_info.format = format;
    view_create_info.subresourceRange.aspectMask = aspectFlags;

    // TODO: Make configurable
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logicalDevice, &view_create_info, context->allocator, &image->view));

};

KINLINE void vulkanImageCreate(
vulkanContext * context, VkImageType imageType, u32 width, u32 height, VkFormat format, VkImageTiling tiling, 
VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags, b32 createView, VkImageAspectFlags viewAspectFlags,
vulkanImage * outImage){

     // Copy params
    outImage->width = width;
    outImage->height = height;

    // Creation info.
    VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;  // TODO: Support configurable depth.
    image_create_info.mipLevels = 4;     // TODO: Support mip mapping
    image_create_info.arrayLayers = 1;   // TODO: Support number of layers in the image.
    image_create_info.format = format;
    image_create_info.tiling = tiling;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;          // TODO: Configurable sample count.
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.

    VK_CHECK(vkCreateImage(context->device.logicalDevice, &image_create_info, context->allocator, &outImage->handle));

    // Query memory requirements.
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(context->device.logicalDevice, outImage->handle, &memory_requirements);

    i32 memory_type = context->findMemoryIndex(memory_requirements.memoryTypeBits, memoryFlags);
    if (memory_type == -1) {
        KERROR("Required memory type not found. Image not valid.");
    }

    // Allocate memory
    VkMemoryAllocateInfo memory_allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type;
    VK_CHECK(vkAllocateMemory(context->device.logicalDevice, &memory_allocate_info, context->allocator, &outImage->memory));

    // Bind the memory
    VK_CHECK(vkBindImageMemory(context->device.logicalDevice, outImage->handle, outImage->memory, 0));  // TODO: configurable memory offset.

    // Create view
    if (createView) {
        outImage->view = 0;
        vulkanImageViewCreate(context, format, outImage, viewAspectFlags);
    }

};



KINLINE void vulkanImageDestroy(vulkanContext * context, vulkanImage * image){

     if (image->view) {
        vkDestroyImageView(context->device.logicalDevice, image->view, context->allocator);
        image->view = 0;
    }
    if (image->memory) {
        vkFreeMemory(context->device.logicalDevice, image->memory, context->allocator);
        image->memory = 0;
    }
    if (image->handle) {
        vkDestroyImage(context->device.logicalDevice, image->handle, context->allocator);
        image->handle = 0;
    }


}

