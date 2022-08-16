#include "rendererTypes.inl"
#include "vulkanBackend.h"
struct platformState;

 b8 vulkanRendererBackendInitialize(rendererBackend* backend, const char* applicationName, struct platformState* plat_state);
 void vulkanRendererBackendShutdown(rendererBackend * backend);
  b8 vulkanRendererBackendBeginFrame(rendererBackend* backend, f32 deltaTime);
 b8 vulkanRendererBackendEndFrame(rendererBackend * backend, f32 deltaTime);
 void vulkanRendererBackendOnResized(rendererBackend * backend, u16 width, u16 height);

 KINLINE b8 rendererBackendCreate(rendererBackendTypes type, struct platformState * platState, rendererBackend 
* outRendererBackend){

outRendererBackend->platState = platState;

if(type == RENDERER_BACKEND_TYPE_VULKAN){
    outRendererBackend->initialize = vulkanRendererBackendInitialize;
    outRendererBackend->shutdown = vulkanRendererBackendShutdown;
    outRendererBackend->beginFrame = vulkanRendererBackendBeginFrame;
    outRendererBackend->endFrame = vulkanRendererBackendEndFrame;
    outRendererBackend->resized = vulkanRendererBackendOnResized;

    return true;
}

return false;
};

KINLINE void rendererBackendDestroy(rendererBackend * renderBackend){

    renderBackend->initialize = 0;
    renderBackend->shutdown = 0;
    renderBackend->beginFrame = 0;
    renderBackend->endFrame = 0;
    renderBackend->resized = 0;

}
