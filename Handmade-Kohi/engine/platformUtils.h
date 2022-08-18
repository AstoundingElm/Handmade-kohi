#pragma once
#include "../defines.h"
#include "platform.h"
#include "vulkanTypes.h"
#include "containers/darray.h"
// Surface creation for Vulkan
KINLINE b8 platform_create_vulkan_surface(vulkanContext* context) {
    if(!platform_state_ptr) {
        return false;
    }

    VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
    create_info.connection = platform_state_ptr->connection;
    create_info.window = platform_state_ptr->window;

    VkResult result = vkCreateXcbSurfaceKHR(
        context->instance,
        &create_info,
        context->allocator,
        &platform_state_ptr->surface);
    if (result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed.");
        return false;
    }
    

    context->contextSurface = platform_state_ptr->surface;
    return true;
}
