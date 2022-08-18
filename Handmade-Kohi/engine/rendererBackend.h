#include "rendererTypes.inl"
#include "vulkanBackend.h"
struct platformState;

 KINLINE b8 vulkanRendererBackendInitialize(renderer_backend* backend, const char* application_name, struct platformState* plat_state);
 KINLINE void vulkanRendererBackendShutdown(renderer_backend * backend);
 KINLINE  b8 vulkanRendererBackendBeginFrame(renderer_backend* backend, f32 deltaTime);
 KINLINE b8 vulkanRendererBackendEndFrame(renderer_backend * backend, f32 deltaTime);
 KINLINE void vulkanRendererBackendOnResized(renderer_backend * backend, u16 width, u16 height);

KINLINE b8 renderer_backend_create(rendererBackendTypes type, renderer_backend* out_renderer_backend) {
    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        out_renderer_backend->initialize = vulkanRendererBackendInitialize;
        out_renderer_backend->shutdown = vulkanRendererBackendShutdown;
        out_renderer_backend->begin_frame = vulkanRendererBackendBeginFrame;
        out_renderer_backend->update_global_state = vulkan_renderer_update_global_state;
        out_renderer_backend->end_frame = vulkanRendererBackendEndFrame;
        out_renderer_backend->resized = vulkanRendererBackendOnResized;

        return true;
    }

    return false;
}

KINLINE void renderer_backend_destroy(renderer_backend * renderBackend){

    renderBackend->initialize = 0;
    renderBackend->shutdown = 0;
    renderBackend->begin_frame = 0;
    renderBackend->update_global_state = 0;
    renderBackend->end_frame = 0;
    renderBackend->resized = 0;

}
