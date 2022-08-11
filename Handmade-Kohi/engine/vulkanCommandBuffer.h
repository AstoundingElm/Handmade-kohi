#include "vulkanTypes.h"
#include "kmemory.h"

static inline void vulkanCommandBufferAllocate(vulkanContext * context, VkCommandPool pool, b8 isPrimary, 
vulkanCommandBuffer * outCommandBuffer){

    kzeroMemory(outCommandBuffer, sizeof(outCommandBuffer));

    VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = pool;
    allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = 0;

    outCommandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logicalDevice,
        &allocateInfo,
        &outCommandBuffer->handle));
    outCommandBuffer->state = COMMAND_BUFFER_STATE_READY;
    

};

static inline void vulkanCommandBufferFree(vulkanContext * context, VkCommandPool pool, vulkanCommandBuffer * commandBuffer){

            vkFreeCommandBuffers(context->device.logicalDevice,
            pool, 1, &commandBuffer->handle);

            commandBuffer->handle = 0;
            commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
            
}

static inline void vulkanCommandBufferBegin(vulkanCommandBuffer * commandBuffer, b8 isSingleRenderUse, 
b8 isRenderPassContinue, b8 isSimultaneousUse){
    
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
      if (isSingleRenderUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (isRenderPassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (isSimultaneousUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    };

      VK_CHECK(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;

}

static inline void vulkanCommandBufferEnd(vulkanCommandBuffer * commandBuffer){

      VK_CHECK(vkEndCommandBuffer(commandBuffer->handle));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;

}

static inline void vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer * commandBuffer){

    commandBuffer->state = COMMAND_BUFFER_STATE_SUBMITTED;

}


static inline void vulkanCommandBufferReset(vulkanCommandBuffer * commandBuffer){

    commandBuffer->state = COMMAND_BUFFER_STATE_READY;

}

static inline void vulkanCommandBufferAllocateAndBeginSingleUse(vulkanContext * context, VkCommandPool pool,
vulkanCommandBuffer * outCommandBuffer){

    vulkanCommandBufferAllocate(context, pool, true, outCommandBuffer);
    vulkanCommandBufferBegin(outCommandBuffer, true, false, false);
}

static inline void vulkanCommandBufferEndSingleUse(vulkanContext * context, VkCommandPool pool, vulkanCommandBuffer 
*commandBuffer, VkQueue queue){

        vulkanCommandBufferEnd(commandBuffer);

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer->handle;
        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, 0));

        VK_CHECK(vkQueueWaitIdle(queue));

        vulkanCommandBufferFree(context, pool, commandBuffer);
}