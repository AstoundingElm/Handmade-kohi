#include "rendererTypes.inl"
#include "platform.h"
#include "kstring.h"
#include "vulkanDevice.cpp"
#include "platformUtils.h"
//#include "vulkan/vulkan_xcb.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

static vulkanContext context;

static inline b8 vulkanRendererBackendInitialize(rendererBackend* backend, const char* applicationName, struct platformState* platState){

context.allocator = 0;

VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
appInfo.pApplicationName = applicationName;
appInfo.applicationVersion = VK_MAKE_VERSION(1, 0 , 0);
appInfo.pEngineName = "Petes Engine";
appInfo.engineVersion = VK_MAKE_VERSION(1, 0 , 0);

const char ** requiredExtensions  = (const char **)darrayCreate(const char *);
darray_push(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME); 
platformGetRequiredExtensionNames(&requiredExtensions);
#if defined(_DEBUG)
darray_push(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

KINFO("Vulkan renderer initialized successfully");
return true; 

//TODO: load vulkan funcs manually;
//signal handler

}
static inline void vulkanRendererBackendShutdown(rendererBackend* backend){

    KDEBUG("Destroying vulkan renderer");
    if(context.debugMessenger){
        PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }
    KDEBUG("Destroying vulkan instance");
    vkDestroyInstance(context.instance, context.allocator);
};

static inline void vulkanRendererBackendOnResized(rendererBackend* backend, u16 width, u16 height){};

extern "C" b8 vulkanRendererBackendBeginFrame(rendererBackend* backend, f32 deltaTime){return true;};
b8 vulkanRendererBackendEndFrame(rendererBackend* backend, f32 deltaTime){return true;};


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