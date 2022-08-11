#pragma once
#include "kmemory.h"
#include "vulkanTypes.h"

static inline void vulkanFrameBufferCreate(vulkanContext * context, vulkanRenderpass * renderPass, u32 width,
u32 height, u32 attachmentCount, VkImageView * attachments, vulkanFrameBuffer * outFrameBuffer){

    outFrameBuffer->attachments = (VkImageView * )kallocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
    for(u32 i = 0; i < attachmentCount; ++i){
        outFrameBuffer->attachments[i] = attachments[i];
    }
    outFrameBuffer->renderPass = renderPass;
    outFrameBuffer->attachmentCount = attachmentCount;

    VkFramebufferCreateInfo frameBufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};

    frameBufferCreateInfo.renderPass = renderPass->handle;
    frameBufferCreateInfo.attachmentCount = attachmentCount;
    frameBufferCreateInfo.pAttachments = outFrameBuffer->attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(
        context->device.logicalDevice,
        &frameBufferCreateInfo,
        context->allocator,
        &outFrameBuffer->handle));
}

static inline void vulkanFrameBufferDestroy(vulkanContext * context, vulkanFrameBuffer * frameBuffer){
 vkDestroyFramebuffer(context->device.logicalDevice, frameBuffer->handle, context->allocator);
    if (frameBuffer->attachments) {
        kfree(frameBuffer->attachments, sizeof(VkImageView) * frameBuffer->attachmentCount, MEMORY_TAG_RENDERER);
        frameBuffer->attachments = 0;
    }
    frameBuffer->handle = 0;
    frameBuffer->attachmentCount = 0;
    frameBuffer->renderPass = 0;
};