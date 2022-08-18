#include "vulkanTypes.h"
#include "kmemory.h"

KINLINE void vulkanCommandBufferAllocate(vulkanContext * context, VkCommandPool pool, b8 isPrimary, 
vulkanCommandBuffer * out_command_buffer){
 kzeroMemory(out_command_buffer, sizeof(out_command_buffer));

    
    kzeroMemory(out_command_buffer, sizeof(out_command_buffer));

    VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate_info.commandPool = pool;
    allocate_info.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = 0;

    out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logicalDevice,
        &allocate_info,
        &out_command_buffer->handle));
    out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
    

};

KINLINE void vulkanCommandBufferFree(vulkanContext * context, VkCommandPool pool, vulkanCommandBuffer * commandBuffer){
 vkFreeCommandBuffers(
        context->device.logicalDevice,
        pool,
        1,
        &commandBuffer->handle);

    commandBuffer->handle = 0;
    commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
            
}

KINLINE void vulkanCommandBufferBegin(vulkanCommandBuffer * command_buffer, b8 is_single_use, 
b8 is_renderpass_continue, b8 is_simultaneous_use){
 VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if (is_single_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (is_renderpass_continue) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (is_simultaneous_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

//vulkanFenceWait()

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;

}

KINLINE void vulkanCommandBufferEnd(vulkanCommandBuffer * command_buffer){

   VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;

}

KINLINE void vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer * commandBuffer){
   commandBuffer->state = COMMAND_BUFFER_STATE_SUBMITTED;

}


KINLINE void vulkanCommandBufferReset(vulkanCommandBuffer * commandBuffer){

    commandBuffer->state = COMMAND_BUFFER_STATE_READY;

}

KINLINE void vulkanCommandBufferAllocateAndBeginSingleUse(vulkanContext * context, VkCommandPool pool,
vulkanCommandBuffer * outCommandBuffer){
    vulkanCommandBufferAllocate(context, pool, true, outCommandBuffer);
    vulkanCommandBufferBegin(outCommandBuffer, true, false, false);
}

KINLINE void vulkanCommandBufferEndSingleUse(vulkanContext * context, VkCommandPool pool, vulkanCommandBuffer 
*commandBuffer, VkQueue queue){

    // End the command buffer.
    vulkanCommandBufferEnd(commandBuffer);

    // Submit the queue
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

    // Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(queue));

    // Free the command buffer.
    vulkanCommandBufferFree(context, pool, commandBuffer);
}