#pragma once
#include "../defines.h"
#include "platform.h"
#include "vulkanTypes.h"
#include "darray.h"
static inline b8 platformCreateVulkanSurface(struct platformState * platState, struct vulkanContext * context){

internalState * state = (internalState *)platState->internalState;

VkXcbSurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
createInfo.connection = state->connection;
createInfo.window = state->window;

VkResult result = vkCreateXcbSurfaceKHR(context->instance, &createInfo, context->allocator, &state->surface);
if(result != VK_SUCCESS){

    KFATAL("Surface creatuon failed");
    return false;
};
context->contextSurface = state->surface;
//createInfo.connection = connection;
return true;

}
