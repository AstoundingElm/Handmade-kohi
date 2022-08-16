#pragma once
#include "kmemory.h"
#include "vulkanTypes.h"

KINLINE void vulkanFrameBufferCreate(vulkanContext * context, vulkanRenderpass * renderPass, u32 width,
u32 height, u32 attachmentCount, VkImageView * attachments, vulkanFrameBuffer * outFramebuffer){

     outFramebuffer->attachments = (VkImageView *)kallocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
    for (u32 i = 0; i < attachmentCount; ++i) {
        outFramebuffer->attachments[i] = attachments[i];
    }
    outFramebuffer->renderPass = renderPass;
    outFramebuffer->attachmentCount = attachmentCount;

    // Creation info
    VkFramebufferCreateInfo framebuffer_create_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_create_info.renderPass = renderPass->handle;
    framebuffer_create_info.attachmentCount = attachmentCount;
    framebuffer_create_info.pAttachments = outFramebuffer->attachments;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height = height;
    framebuffer_create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(
        context->device.logicalDevice,
        &framebuffer_create_info,
        context->allocator,
        &outFramebuffer->handle));

    }

KINLINE void vulkanFrameBufferDestroy(vulkanContext * context, vulkanFrameBuffer * frameBuffer){
  vkDestroyFramebuffer(context->device.logicalDevice, frameBuffer->handle, context->allocator);
    if (frameBuffer->attachments) {
        kfree(frameBuffer->attachments, sizeof(VkImageView) * frameBuffer->attachmentCount, MEMORY_TAG_RENDERER);
        frameBuffer->attachments = 0;
    }
    frameBuffer->handle = 0;
    frameBuffer->attachmentCount = 0;
    frameBuffer->renderPass = 0;
};