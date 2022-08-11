#include "../defines.h"
#include "vulkanTypes.h"
#include "kmemory.h"

#define ATTACHMENT_DESCRIPTION_COUNT 2

static void vulkanRenderPassCreate(vulkanContext * context, vulkanRenderpass * outRenderpass, f32 x, f32 y, 
f32 w, f32 h, f32 r, f32 g,f32 b, f32 a, f32 depth, u32 stencil){

     VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Attachments TODO: make this configurable.
    u32 attachment_description_count = 2;
    VkAttachmentDescription attachment_descriptions[attachment_description_count];

    // Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = context->swapchain.imageFormat.format; // TODO: configurable
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Do not expect any particular layout before render pass starts.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Transitioned to after the render pass
    colorAttachment.flags = 0;    attachment_descriptions[0] = colorAttachment;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;  // Attachment description array index
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    // Depth attachment, if there is one
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = context->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachment_descriptions[1] = depthAttachment;

    // Depth attachment reference
    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // TODO: other attachment types (input, resolve, preserve)

    // Depth stencil data.
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    // Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    // Attachments used for multisampling colour attachments
    subpass.pResolveAttachments = 0;

    // Attachments not used in this subpass, but must be preserved for the next.
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    // Render pass dependencies. TODO: make this configurable.
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    // Render pass create.
    VkRenderPassCreateInfo renderpassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderpassCreateInfo.attachmentCount = attachment_description_count;
    renderpassCreateInfo.pAttachments = attachment_descriptions;
    renderpassCreateInfo.subpassCount = 1;
    renderpassCreateInfo.pSubpasses = &subpass;
    renderpassCreateInfo.dependencyCount = 1;
    renderpassCreateInfo.pDependencies = &dependency;
    renderpassCreateInfo.pNext = 0;
    renderpassCreateInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        context->device.logicalDevice,
        &renderpassCreateInfo,
        context->allocator,
        &outRenderpass->handle));

};

static inline void vulkanRenderpassDestroy(vulkanContext * context, vulkanRenderpass * renderPass){

     if (renderPass && renderPass->handle) {
        vkDestroyRenderPass(context->device.logicalDevice, renderPass->handle, context->allocator);
        renderPass->handle = 0;
    }

}

static inline void vulkanRenderPassBegin(vulkanCommandBuffer * commandBuffer, vulkanRenderpass * renderPass, 
VkFramebuffer frameBuffer){

     VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = renderPass->handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderPass->x;
    beginInfo.renderArea.offset.y = renderPass->y;
    beginInfo.renderArea.extent.width = renderPass->w;
    beginInfo.renderArea.extent.height = renderPass->h;

    VkClearValue clearValues[2];
    kzeroMemory(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = renderPass->r;
    clearValues[0].color.float32[1] = renderPass->g;
    clearValues[0].color.float32[2] = renderPass->b;
    clearValues[0].color.float32[3] = renderPass->a;
    clearValues[1].depthStencil.depth = renderPass->depth;
    clearValues[1].depthStencil.stencil = renderPass->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;


}

static inline void vulkanRenderPassEnd(vulkanCommandBuffer * commandBuffer, vulkanRenderpass * renderPass){

vkCmdEndRenderPass(commandBuffer->handle);
commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;

};
