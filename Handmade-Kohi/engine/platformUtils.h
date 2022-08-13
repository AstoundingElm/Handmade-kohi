#pragma once
#include "../defines.h"
#include "platform.h"
#include "vulkanTypes.h"
#include "darray.h"
KINLINE b8 platformCreateVulkanSurface(struct platformState * plat_state, struct vulkanContext * context){

   internal_state *state = (internal_state *)plat_state->internal_state;

    VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
    create_info.connection = state->connection;
    create_info.window = state->window;

    VkResult result = vkCreateXcbSurfaceKHR(
        context->instance,
        &create_info,
        context->allocator,
        &state->surface);
    if (result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed.");
        return false;
    }

    context->contextSurface = state->surface;
    return true;

}
