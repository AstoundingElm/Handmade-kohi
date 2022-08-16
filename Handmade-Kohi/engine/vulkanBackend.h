#include "../defines.h"
#include "rendererTypes.inl"
#include "vulkanFrameBuffer.h"
#include "platform.h"
#include "kstring.h"
#include "vulkanDevice.h"
#include "platformUtils.h"
#include "vulkanSwapchain.h"
#include "vulkanRenderPass.cpp"
//#include "vulkanCommandBuffer.h"
#include "vulkanFence.h"
#include "vulkanUtils.h"
#include "../shaders/vulkan_object_shader.h"
#include "mathTypes.h"
#include "vulkan_buffer.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);
 
static i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags);


static vulkanContext context;
static u32 cachedFramebufferWidth = 0;
static u32 cachedFramebufferHeight = 0;

//VkBufferUsageFlagBits)
b8 create_buffers(vulkanContext * context){
 
 VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 vertex_buffer_size = sizeof(vertex_3d) * 1024 * 1024;
    if (!vulkan_buffer_create(
            context,
            vertex_buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memory_property_flags,
            true,
            &context->object_vertex_buffer)) {
        KERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometry_vertex_offset = 0;

    const u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!vulkan_buffer_create(
            context,
            index_buffer_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memory_property_flags,
            true,
            &context->object_index_buffer)) {
        KERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometry_index_offset = 0;

    return true;
};

void upload_data_range(vulkanContext * context, VkCommandPool pool, VkFence fence, VkQueue queue, vulkan_buffer * buffer,   u64 offset,
u64 size, void * data){

    VkBufferUsageFlags  flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    vulkan_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    vulkan_buffer_load_data(context, &staging, 0, size, 0, data);
    vulkan_buffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
    vulkan_buffer_destroy(context, &staging);
}
KINLINE void createCommandBuffers(rendererBackend * backend){

   if (!context.graphicsCommandBuffers) {
        context.graphicsCommandBuffers = (vulkanCommandBuffer *)darray_reserve(vulkanCommandBuffer, context.swapchain.imageCount);
        for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
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
};



KINLINE void regenerateFrameBuffers(rendererBackend* backend, vulkanSwapchain * swapchain, 
vulkanRenderpass * renderPass){ for (u32 i = 0; i < swapchain->imageCount; ++i) {
        // TODO: make this dynamic based on the currently configured attachments
        u32 attachment_count = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depthAttachment.view};

        vulkanFrameBufferCreate(
            &context,
            renderPass,
            context.framebufferWidth,
            context.framebufferHeight,
            attachment_count,
            attachments,
            &context.swapchain.frameBuffers[i]);
    }

}

void platform_get_required_extension_names(const char ***names_darray) {
    darray_push(*names_darray, &"VK_KHR_xcb_surface");  // VK_KHR_xlib_surface?
}

KINLINE b8 recreateSwapchain(rendererBackend * backend){

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
        vulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkanFrameBufferDestroy(&context, &context.swapchain.frameBuffers[i]);
    }

    context.mainRenderPass.x = 0;
    context.mainRenderPass.y = 0;
    context.mainRenderPass.w = context.framebufferWidth;
    context.mainRenderPass.h = context.framebufferHeight;

    regenerateFrameBuffers(backend, &context.swapchain, &context.mainRenderPass);

    createCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreatingSwapchain = false;

    return true;
}

static void applicationGetFramebufferSize(u32 * width, u32 * height);

KINLINE b8 vulkanRendererBackendInitialize(rendererBackend* backend, const char* applicationName, struct platformState* plat_state){

 context.findMemoryIndex = findMemoryIndex;

    // TODO: custom allocator.
    context.allocator = 0;

    applicationGetFramebufferSize(&cachedFramebufferWidth, &cachedFramebufferHeight);
    context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Setup Vulkan instance.
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = applicationName;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Kohi Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** required_extensions = (const char **)darrayCreate(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);  // Generic surface extension
  //  platformGetRequiredExtensionNames(&required_extensions);  
       // Platform-specific extension(s)
       // darray_push(*required_extensions, &"VK_KHR_xcb_surface");

        platform_get_required_extension_names(&required_extensions);    // debug utilities
 darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); 
    KDEBUG("Required extensions:");
    u32 length = darrayLength(required_extensions);
    for (u32 i = 0; i < length; ++i) {
        KDEBUG(required_extensions[i]);
    }


    create_info.enabledExtensionCount = darrayLength(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    // Validation layers.
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

// If validation should be done, get a list of the required validation layert names
// and make sure they exist. Validation layers should only be enabled on non-release builds.

    KINFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    required_validation_layer_names =(const char**) darrayCreate(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    required_validation_layer_count = darrayLength(required_validation_layer_names);

    // Obtain a list of available validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = (VkLayerProperties *)darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available.
    for (u32 i = 0; i < required_validation_layer_count; ++i) {
        KINFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = false;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (stringsEqual(required_validation_layer_names[i], available_layers[j].layerName)) {
                found = true;
                KINFO("Found.");
                break;
            }
        }

        if (!found) {
            KFATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return true;
        }
    }
    KINFO("All required validation layers are present.");


    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

   // #define VK_NO_PROTOTYPES



    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    KINFO("Vulkan Instance created.");

    // Debugger

    KDEBUG("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vkDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    KASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debugMessenger));
    KDEBUG("Vulkan debugger created.");


    // Surface
    KDEBUG("Creating Vulkan surface...");
    if (!platformCreateVulkanSurface(plat_state, &context)) {
        KERROR("Failed to create platform surface!");
        return false;
    }
    KDEBUG("Vulkan surface created.");

    // Device creation
    if (!vulkanDeviceCreate(&context)) {
        KERROR("Failed to create device!");
        return false;
    }

    // Swapchain
    vulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain);

    vulkanRenderPassCreate(
        &context,
        &context.mainRenderPass,
        0, 0, context.framebufferWidth, context.framebufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0);

    // Swapchain framebuffers.
    context.swapchain.frameBuffers = (vulkanFrameBuffer *)darray_reserve(vulkanFrameBuffer, context.swapchain.imageCount);
    regenerateFrameBuffers(backend, &context.swapchain, &context.mainRenderPass);

    // Create command buffers.
    createCommandBuffers(backend);

    // Create sync objects.
    context.imageAvailableSemaphores = (VkSemaphore *)darray_reserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = (VkSemaphore *)darray_reserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = (vulkanFence * )darray_reserve(vulkanFence, context.swapchain.maxFramesInFlight);

    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.queueCompleteSemaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        vulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
    // by this list.
    context.imagesInFlight = (vulkanFence **)darray_reserve(vulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    };

    if(!vulkan_object_shader_create(&context, &context.object_shader)){

        KERROR(" Error loading builtin basic_lighting shader ");
        return false;   
    };

    create_buffers(&context);
    
    const u32 vert_count = 4;
    vertex_3d verts[vert_count];
    kzeroMemory(verts, sizeof(vertex_3d) * vert_count);
    verts[0].position.x = 0.0;
    verts[0].position.y = -0.5;

    verts[1].position.x = 0.5;
    verts[1].position.y = 0.5;

    verts[2].position.x = 0;
    verts[2].position.y = 0.5;

    verts[3].position.x = 0.5;
    verts[3].position.y = -0.5;

    const u32 index_count = 6;
    u32 indices[index_count] = {0, 1, 2, 0, 3, 1};

    upload_data_range(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.object_vertex_buffer, 0, sizeof(vertex_3d) * vert_count, verts);
    upload_data_range(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.object_index_buffer, 0, sizeof(u32) * index_count, indices);

    KINFO("Vulkan renderer initialized successfully.");
    return true; 


};
KINLINE void vulkanRendererBackendShutdown(rendererBackend* backend){
    vkDeviceWaitIdle(context.device.logicalDevice);

    vulkan_object_shader_destroy(&context, &context.object_shader); 

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
    KDEBUG("testingtesting testeing");
    vkDestroyInstance(context.instance, context.allocator);
};

KINLINE void vulkanRendererBackendOnResized(rendererBackend* backend, u16 width, u16 height){

    cachedFramebufferWidth = width;
    cachedFramebufferHeight = height;
    context.framebufferSizeGeneration++;

    KINFO("Vulkan renderer backend->resized: w/h/gen: %i%i%llu", width, height, context.framebufferSizeGeneration);

};

extern "C" b8 vulkanRendererBackendBeginFrame(rendererBackend* backend, f32 deltaTime){
    
vulkanDevice* device = &context.device;

    // Check if recreating swap chain and boot out.
    if (context.recreatingSwapchain) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!vulkanResultIsSuccess(result)) {
            KERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", vulkanResultString(result, true));
            return false;
        }
        KINFO("Recreating swapchain, booting.");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!vulkanResultIsSuccess(result)) {
            KERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: '%s'", vulkanResultString(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // boot out before unsetting the flag.
        if (!recreateSwapchain(backend)) {
            return false;
        }

        KINFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
    if (!vulkanFenceWait(
            &context,
            &context.inFlightFences[context.currentFrame],
            UINT64_MAX)) {
        KWARN("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
    // This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if (!vulkanSwapchainAquireNextImageIndex(
            &context,
            &context.swapchain,
            UINT64_MAX,
            context.imageAvailableSemaphores[context.currentFrame],
            0,
            &context.imageIndex)) {
        return false;
    }

    // Begin recording commands.
    vulkanCommandBuffer* command_buffer = &context.graphicsCommandBuffers[context.imageIndex];
    vulkanCommandBufferReset(command_buffer);
    vulkanCommandBufferBegin(command_buffer, false, false, false);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebufferHeight;
    viewport.width = (f32)context.framebufferWidth;
    viewport.height = -(f32)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.mainRenderPass.w = context.framebufferWidth;
    context.mainRenderPass.h = context.framebufferHeight;

    // Begin the render pass.
    vulkanRenderPassBegin(
        command_buffer,
        &context.mainRenderPass,
        context.swapchain.frameBuffers[context.imageIndex].handle);

        vulkan_object_shader_use(&context, &context.object_shader);
 
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, (VkDeviceSize *)offsets);
        vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);

    return true;



   };


static b8 vulkanRendererBackendEndFrame(rendererBackend* backend, f32 deltaTime){
    
     vulkanCommandBuffer* command_buffer = &context.graphicsCommandBuffers[context.imageIndex];

    // End renderpass
    vulkanRenderPassEnd(command_buffer, &context.mainRenderPass);

    vulkanCommandBufferEnd(command_buffer);

    // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) {  // was frame
        vulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            UINT64_MAX);
    }

    // Mark the image fence as in-use by this frame.
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    // Reset the fence for use on the next frame
    vulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    // Submit the queue and wait for the operation to complete.
    // Begin queue submission
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed.
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    // The semaphore(s) to be signaled when the queue is complete.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    // Wait semaphore ensures that the operation cannot begin until the image is available.
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue,
        1,
        &submit_info,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS) {
        KERROR("vkQueueSubmit failed with result: %s", vulkanResultString(result, true));
        return false;
    }

    vulkanCommandBufferUpdateSubmitted(command_buffer);
    // End queue submission

    // Give the image back to the swapchain.
    vulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);


    return true;

   };


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

static i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags) {
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


