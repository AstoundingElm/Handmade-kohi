#include "rendererTypes.inl"
#include "vulkanFrameBuffer.h"
#include "platform.h"
#include "kstring.h"
#include "vulkanDevice.h"
#include "platformUtils.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderPass.cpp"
#include "vulkanCommandBuffer.h"
#include "vulkanFence.h"
#include "vulkanUtils.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);
 
i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags);


static vulkanContext context;
static u32 cachedFramebufferWidth = 0;
static u32 cachedFramebufferHeight = 0;



static inline void createCommandBuffers(rendererBackend * backend){

    if(!context.graphicsCommandBuffers){

        context.graphicsCommandBuffers = (vulkanCommandBuffer *)darray_reserve(vulkanCommandBuffer, context.swapchain.imageCount);
        for(u32 i = 0; i <context.swapchain.imageCount; ++i){
        kzeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkanCommandBuffer));

        }
    }

    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
        }
        kzeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkanCommandBuffer));
        vulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            true,
            &context.graphicsCommandBuffers[i]);
    }

    KDEBUG("Vulkan command buffers created.");

}

static inline b8 recreateSwapchain(rendererBackend * backend){

    if(context.recreatingSwapchain){

        KDEBUG("recreateSwapchain called when already recreating. Booting.");
        return false;
    }

      if (context.framebufferWidth == 0 || context.framebufferHeight == 0) {
        KDEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    // Mark as recreating if the dimensions are valid.
    context.recreatingSwapchain = true;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logicalDevice);

    // Clear these out just in case.
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    // Requery support
    vulkanDeviceQuerySwapchainSupport(
        context.device.physicalDevice,
        context.contextSurface,
        &context.device.swapchainSupport);
    vulkanDeviceDetectDepthFormat(&context.device);

    vulkanSwapchainRecreate(
        &context,
        cachedFramebufferWidth,
        cachedFramebufferHeight,
        &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebufferWidth = cachedFramebufferWidth;
    context.framebufferHeight = cachedFramebufferHeight;
    context.mainRenderPass.w = context.framebufferWidth;
    context.mainRenderPass.h = context.framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Update framebuffer size generation.
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    // cleanup swapchain
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkan_command_buffer_free(&context, context.device.graphicsCommandPool, &context.graphics_command_buffers[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_framebuffer_destroy(&context, &context.swapchain.frameBuffers[i]);
    }

    context.mainRenderPass.x = 0;
    context.mainRenderPass.y = 0;
    context.mainRenderPass.w = context.framebufferWidth;
    context.mainRenderPass.h = context.framebufferHeight;

    regenerateFramebuffers(backend, &context.swapchain, &context.mainRenderPass);

    createCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreatingWwapchain = false;

    return true;
}

static inline void regenerateFrameBuffers(rendererBackend* backend, vulkanSwapchain * swapchain, 
vulkanRenderpass * renderPass){

    for(u32 i = 0; i < swapchain->imageCount; ++i ){

        u32 attachmentCount = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depthAttachment.view};

        vulkanFrameBufferCreate(
            &context,
            renderPass,
            context.framebufferWidth,
            context.framebufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.frameBuffers[i]);
    }
        

}
void applicationGetFramebufferSize(u32 * width, u32 * height);

static inline b8 vulkanRendererBackendInitialize(rendererBackend* backend, const char* applicationName, struct platformState* platState){

context.findMemoryIndex = findMemoryIndex;

context.allocator = 0;

applicationGetFramebufferSize(&cachedFramebufferHeight, &cachedFramebufferWidth);
  context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
appInfo.pApplicationName = applicationName;
appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
appInfo.pEngineName = "Petes Engine";
appInfo.engineVersion = VK_MAKE_VERSION(1, 0 , 0);

const char ** requiredExtensions  = (const char **)darrayCreate(const char *);
darray_push(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME); 

darray_push(requiredExtensions, &"VK_KHR_xcb_surface");
darray_push(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#if defined(_DEBUG)
KDEBUG("Required extensions:");
u32 length = darrayLength(requiredExtensions);
for(u32 i = 0; i < length; ++i){

    KDEBUG(requiredExtensions[i]);

}
#endif

VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
createInfo.pApplicationInfo = &appInfo;
createInfo.enabledExtensionCount = darrayLength(requiredExtensions);//arraySize(requiredExtensions);//darrayLength(requiredExtensions);
createInfo.ppEnabledExtensionNames = requiredExtensions;

const char ** requiredValidationLayerNames = 0;
u32 requiredValidationLayerCount = 0;


#if defined(_DEBUG)
KINFO("Validation layers enabled. Enumerating...");
requiredValidationLayerNames = (const char **)darrayCreate(const char *);
darray_push(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
requiredValidationLayerCount = darrayLength(requiredValidationLayerNames);

u32 availableLayerCount = 0;

VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
VkLayerProperties *availableLayers = (VkLayerProperties *)darray_reserve(VkLayerProperties, availableLayerCount);
VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

for(u32 i = 0; i < requiredValidationLayerCount; ++i){

    KINFO("Searching for layer: ... %s", requiredValidationLayerNames[i]);
    b8 found = false;
    for(u32 j = 0; j < availableLayerCount; ++j){
        if(stringsEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)){
            found = true;
            KINFO("Found.");
            break;

        }
    }
    if(!found){

        KFATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
        return false;
    }
}
KINFO("All required validation layers are present.");
#endif
createInfo.enabledLayerCount = requiredValidationLayerCount;
createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
KINFO("Vulkan instance successfully created");

#if defined(_DEBUG)
KDEBUG("Creating vulkan debugger");
u32 logSeverity=  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT|
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo  = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
debugCreateInfo.messageSeverity = logSeverity;
debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
debugCreateInfo.pfnUserCallback = vkDebugCallback;
PFN_vkCreateDebugUtilsMessengerEXT func = 
(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
KASSERT_MSG(func, "failed to create debug messenger");
VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
KDEBUG("Vulkan debugger successfully created.");

#endif

KDEBUG("Creating vulkan surface");

    if(!platformCreateVulkanSurface(platState, &context)){

        KERROR("Failed to create platform surface");
        return false;

    };

    KDEBUG("Vulkan platform surface successfully created.");
 

if(!vulkanDeviceCreate(&context)){
    KERROR("Failed to create device");
    return false;
}

vulkanSwapchainCreate(&context, context.framebufferWidth, context.framebufferHeight, &context.swapchain);

vulkanRenderPassCreate(&context, &context.mainRenderPass, 0, 0, context.framebufferWidth, context.framebufferHeight,
0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0); 

context.swapchain.frameBuffers = (vulkanFrameBuffer *)darray_reserve(vulkanFrameBuffer, context.swapchain.imageCount);
regenerateFrameBuffers(backend, &context.swapchain, &context.mainRenderPass);

createCommandBuffers(backend);

context.imageAvailableSemaphores = (VkSemaphore* )darray_reserve(VkSemaphore, context.swapchain.maxFramesInFlight);
context.queueCompleteSemaphores = (VkSemaphore *)darray_reserve(VkSemaphore, context.swapchain.maxFramesInFlight);
context.inFlightFences = ( vulkanFence *)darray_reserve( vulkanFence, context.swapchain.maxFramesInFlight);


    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.queueCompleteSemaphores[i]);

        vulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    };

     context.imagesInFlight = (vulkanFence **)darray_reserve(vulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

KINFO("Vulkan renderer initialized successfully");
return true; 

//TODO: load vulkan funcs manually;
//signal handler


};
static inline void vulkanRendererBackendShutdown(rendererBackend* backend){
    vkDeviceWaitIdle(context.device.logicalDevice);

     for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        if (context.imageAvailableSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if (context.queueCompleteSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }
        vulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }
    darray_destroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    darray_destroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    darray_destroy(context.inFlightFences);
    context.inFlightFences = 0;

    darray_destroy(context.imagesInFlight);
    context.imagesInFlight = 0;

     // Command buffers
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }
    darray_destroy(context.graphicsCommandBuffers);
    context.graphicsCommandBuffers = 0;

       // Destroy framebuffers
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkanFrameBufferDestroy(&context, &context.swapchain.frameBuffers[i]);
    }

    vulkanRenderpassDestroy(&context, &context.mainRenderPass);

 // Swapchain
    vulkanSwapchainDestroy(&context, &context.swapchain);

    KDEBUG("Destroying Vulkan device...");
    vulkanDeviceDestroy(&context);

    KDEBUG("Destroying Vulkan surface...");
    if (context.contextSurface) {
        vkDestroySurfaceKHR(context.instance, context.contextSurface, context.allocator);
        context.contextSurface = 0;
    }

    KDEBUG("Destroying vulkan renderer");
    if(context.debugMessenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }
    KDEBUG("Destroying vulkan instance");
    vkDestroyInstance(context.instance, context.allocator);
};

static inline void vulkanRendererBackendOnResized(rendererBackend* backend, u16 width, u16 height){

    cachedFramebufferWidth = width;
    cachedFramebufferHeight = height;
    context.framebufferSizeGeneration++;

    KINFO("Vulkan renderer backend->resized: w/h/gen: %i%i%llu", width, height, context.framebufferSizeGeneration);

};

extern "C" b8 vulkanRendererBackendBeginFrame(rendererBackend* backend, f32 deltaTime){
    
    vulkanDevice * device  = &context.device;
    if(context.recreatingSwapchain){

        VkResult result  = vkDeviceWaitIdle(device->logicalDevice);
        if(!vulkanResultIsSuccess(result)){

            KERROR("vulkanRendererBackendBeginFrame  vkDeviceWaitIdle (1) failed: '%s'", vulkanResultString(result, true));
            return false;
        }
        KINFO("recreating swapchain, booting");
        return false;


    }
    if(context.framebufferSizeGeneration != context.framebufferSizeLastGeneration){

        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if(!vulkanResultIsSuccess(result)){

            KERROR("vulkanRendererBackendBeginFame vkDeviceWaitIdle (2) failed: '%s'", vulkanResultString(result,
            true));
            return false;
        }
        if(!recreateSwapchain(backend)){

                return false;
        }
        KINFO("resized, booting");
        return false;
    }

    if(!vulkanFenceWait(&context, &context.inFlightFences[context.currentFrame], UINT64_MAX)){

        KWARN("Inflight fence wait faulure");
        return false;
    }

    if(!vulkanSwapchainAquireNextImageIndex(&context, &context.swapchain, UINT64_MAX, context.imageAvailableSemaphores
    [context.currentFrame], 0, &context.imageIndex)){

        return false;
    }
    
    vulkanCommandBuffer * commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    vulkanCommandBufferReset(commandBuffer);
    vulkanCommandBufferBegin(commandBuffer, false, false, false);

    VkViewport viewPort;
    viewPort.x = 0.0f;
    viewPort.y = (f32)context.framebufferHeight;
    viewPort.width = (f32)context.framebufferWidth;
    viewPort.height = (f32)context.framebufferHeight;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewPort);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderPass.w = context.framebufferWidth;
    context.mainRenderPass.h = context.framebufferHeight;

    vulkanRenderPassBegin(commandBuffer, &context.mainRenderPass, context.swapchain.frameBuffers[context.
    imageIndex].handle);

    return true;};


b8 vulkanRendererBackendEndFrame(rendererBackend* backend, f32 deltaTime){
    
    vulkanCommandBuffer * commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    vulkanRenderPassEnd(commandBuffer, &context.mainRenderPass);

    if(context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE){

        vulkanFenceWait(&context, context.imagesInFlight[context.imageIndex],UINT64_MAX);
    }

    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    vulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

     submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue, 
        1,
        &submitInfo,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS) {
        KERROR("vkQueueSubmit failed with result: %s", vulkanResultString(result, true));
        return false;
    }

    vulkanCommandBufferUpdateSubmitted(commandBuffer);
    // End queue submission

    // Give the image back to the swapchain.
    vulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);

    return true;};


VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            KERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KWARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            KINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            KTRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);

    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        // Check each memory type to see if its bit is set to 1.
        if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
            return i;
        }
    }

    KWARN("Unable to find suitable memory type!");
    return -1;
}


