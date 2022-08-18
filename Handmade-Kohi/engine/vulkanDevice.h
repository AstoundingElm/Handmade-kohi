#pragma once 
#include "../defines.h"
#include "vulkanTypes.h"
#include "logger.h"
#include "kstring.h"
#include "containers/darray.h" 

 struct vulkanPhysicalDeviceRequirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** deviceExtensionNames;
    b8 samplerAnisotropy;
    b8 discreteGpu;
};

struct vulkanPhysicalDeviceQueueFamilyInfo {
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
};

KINLINE b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkanPhysicalDeviceRequirements* requirements,
    vulkanPhysicalDeviceQueueFamilyInfo* outQueueInfo,
    vulkanSwapchainSupportInfo* outSwapchainSupport);

KINLINE void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    vulkanSwapchainSupportInfo* outSupportInfo) {

        
    // Surface capabilities
   VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &outSupportInfo->formatCount,
        0));

    if (outSupportInfo->formatCount != 0) {
        if (!outSupportInfo->formats) {
            outSupportInfo->formats = (VkSurfaceFormatKHR *)kallocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &outSupportInfo->formatCount,
            outSupportInfo->formats));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->presentModeCount,
        0));
    if (outSupportInfo->presentModeCount != 0) {
        if (!outSupportInfo->presentModes) {
            outSupportInfo->presentModes = (VkPresentModeKHR *)kallocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &outSupportInfo->presentModeCount,
            outSupportInfo->presentModes));
    }
}

KINLINE b8 selectPhysicalDevice(vulkanContext * context){



    u32 physicalDevice_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDevice_count, 0));
    if (physicalDevice_count == 0) {
        KFATAL("No devices which support Vulkan were found.");
        return false;
    }
    const u32 max_device_count = 32;
    VkPhysicalDevice physicalDevices[max_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDevice_count, physicalDevices));
    for (u32 i = 0; i < physicalDevice_count; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
       vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        // TODO: These requirements should probably be driven by engine
        // configuration.
        vulkanPhysicalDeviceRequirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = TRUE;
        requirements.samplerAnisotropy = true;
        requirements.discreteGpu = false;
        requirements.deviceExtensionNames = (const char **)darrayCreate(const char*);
        darray_push(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkanPhysicalDeviceQueueFamilyInfo queue_info = {};
        b8 result = physicalDeviceMeetsRequirements(
            physicalDevices[i],
            context->contextSurface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchainSupport);

        if (result) {
            KINFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    KINFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    KINFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    KINFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    KINFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    KINFO("GPU type is CPU.");
                    break;
            }

            KINFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            // Vulkan API version.
            KINFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            // Memory information
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    KINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                } else {
                    KINFO("Shared System memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physicalDevice = physicalDevices[i];
            context->device.graphicsQueueIndex = queue_info.graphicsFamilyIndex;
            context->device.presentQueueIndex = queue_info.presentFamilyIndex;
            context->device.transferQueueIndex = queue_info.transferFamilyIndex;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }

    // Ensure a device was selected
    if (!context->device.physicalDevice) {
        KERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    KINFO("Physical device selected.");
    return true;
};

KINLINE b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkanPhysicalDeviceRequirements* requirements,
    vulkanPhysicalDeviceQueueFamilyInfo* outQueueInfo,
    vulkanSwapchainSupportInfo* outSwapchainSupport) {
    // Evaluate device properties to determine if it meets the needs of our applcation.
   outQueueInfo->graphicsFamilyIndex = -1;
   outQueueInfo->presentFamilyIndex = -1;
   outQueueInfo->computeFamilyIndex = -1;
   outQueueInfo->transferFamilyIndex = -1;

    // Discrete GPU?
    if (requirements->discreteGpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            KINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Look at each queue and see what queues it supports
    KINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        // Graphics queue?
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            outQueueInfo->graphicsFamilyIndex = i;
            ++current_transfer_score;
        }

        // Compute queue?
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            outQueueInfo->computeFamilyIndex = i;
            ++current_transfer_score;
        }

        // Transfer queue?
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                outQueueInfo->transferFamilyIndex = i;
            }
        }

        // Present queue?
      
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if (supports_present) {
            outQueueInfo->presentFamilyIndex = i;
        }
    }

    // Print out some info about the device
    KINFO("       %d |       %d |       %d |        %d | %s",
          outQueueInfo->graphicsFamilyIndex != -1,
          outQueueInfo->presentFamilyIndex != -1,
          outQueueInfo->computeFamilyIndex != -1,
          outQueueInfo->transferFamilyIndex != -1,
          properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outQueueInfo->transferFamilyIndex != -1))) {
        KINFO("Device meets queue requirements.");
        KTRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex);
        KTRACE("Present Family Index:  %i", outQueueInfo->presentFamilyIndex);
        KTRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex);
        KTRACE("Compute Family Index:  %i", outQueueInfo->computeFamilyIndex);

        // Query swapchain support.
        vulkanDeviceQuerySwapchainSupport(
            device,
            surface,
            outSwapchainSupport);

        if (outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModeCount < 1) {
            if (outSwapchainSupport->formats) {
                kfree(outSwapchainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
            }
            if (outSwapchainSupport->presentModes) {
                kfree(outSwapchainSupport->presentModes, sizeof(VkPresentModeKHR) * outSwapchainSupport->presentModeCount, MEMORY_TAG_RENDERER);
            }
            KINFO("Required swapchain support not present, skipping device.");
            return false;
        }

        // Device extensions.
        if (requirements->deviceExtensionNames) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &available_extension_count,
                0));
            if (available_extension_count != 0) {
                available_extensions = (VkExtensionProperties * )kallocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &available_extension_count,
                    available_extensions));

                u32 required_extension_count = darrayLength(requirements->deviceExtensionNames);
                for (u32 i = 0; i < required_extension_count; ++i) {
                    b8 found = false;
                    for (u32 j = 0; j < available_extension_count; ++j) {
                        if (stringsEqual(requirements->deviceExtensionNames[i], available_extensions[j].extensionName)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        KINFO("Required extension not found: '%s', skipping device.", requirements->deviceExtensionNames[i]);
                        kfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            kfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
            KINFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        // Device meets all requirements.
        return true;
    }

    return false;
};


KINLINE b8 vulkanDeviceCreate(vulkanContext * context){

if (!selectPhysicalDevice(context)) {
        return false;
    }

    KINFO("Creating logical device...");
    // NOTE: Do not create additional queues for shared indices.
    b8 present_shares_graphicsQueue = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
    b8 transfer_shares_graphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
    u32 index_count = 1;
    if (!present_shares_graphicsQueue) {
        index_count++;
    }
    if (!transfer_shares_graphicsQueue) {
        index_count++;
    }
    u32 indices[32];
    u8 index = 0;
    indices[index++] = context->device.graphicsQueueIndex;
    if (!present_shares_graphicsQueue) {
        indices[index++] = context->device.presentQueueIndex;
    }
    if (!transfer_shares_graphicsQueue) {
        indices[index++] = context->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queue_create_infos[32];
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        // TODO: Enable this for a future enhancement.
        // if (indices[i] == context->device.graphicsQueue_index) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    // Deprecated and ignored, so pass nothing.
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    // Create the device.
    VK_CHECK(vkCreateDevice(
        context->device.physicalDevice,
        &device_create_info,
        context->allocator,
        &context->device.logicalDevice));

    KINFO("Logical device created.");

    // Get queues.
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.graphicsQueueIndex,
        0,
        &context->device.graphicsQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.presentQueueIndex,
        0,
        &context->device.presentQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.transferQueueIndex,
        0,
        &context->device.transferQueue);
    KINFO("Queues obtained.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphicsQueueIndex;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        context->device.logicalDevice,
        &pool_create_info,
        context->allocator,
        &context->device.graphicsCommandPool));
    KINFO("Graphics command pool created.");

    return true;
};

 KINLINE void vulkanDeviceDestroy(vulkanContext * context){
  
  context->device.graphicsQueue = 0;
    context->device.presentQueue = 0;
    context->device.transferQueue = 0;

    KINFO("Destroying command pools...");
    vkDestroyCommandPool(
        context->device.logicalDevice,
        context->device.graphicsCommandPool,
        context->allocator);

    // Destroy logical device
    KINFO("Destroying logical device...");
    if (context->device.logicalDevice) {
        vkDestroyDevice(context->device.logicalDevice, context->allocator);
        context->device.logicalDevice = 0;
    }

    // Physical devices are not destroyed.
    KINFO("Releasing physical device resources...");
    context->device.physicalDevice = 0;

    if (context->device.swapchainSupport.formats) {
        kfree(
            context->device.swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.formats = 0;
        context->device.swapchainSupport.formatCount = 0;
    }

    if (context->device.swapchainSupport.presentModes) {
        kfree(
            context->device.swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * context->device.swapchainSupport.presentModeCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.presentModes = 0;
        context->device.swapchainSupport.presentModeCount = 0;
    }

    kzeroMemory(
        &context->device.swapchainSupport.capabilities,
        sizeof(context->device.swapchainSupport.capabilities));

    context->device.graphicsQueueIndex = -1;
    context->device.presentQueueIndex = -1;
    context->device.transferQueueIndex = -1;

};

KINLINE b8 vulkanDeviceDetectDepthFormat(vulkanDevice * device){
    
    const u64 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidate_count; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return true;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return true;
        }
    }

    return false;
}