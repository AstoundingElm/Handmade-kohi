#include "rendererTypes.inl"
#include "rendererBackend.h"
#include "logger.h"
#include "kmemory.h"
struct StaticMeshData;  
struct platformState;

static rendererBackend * backend = 0;
static inline b8 rendererInitialize(const char * applicationName, struct platformState * platState){

backend = (rendererBackend* )kallocate(sizeof(rendererBackend), MEMORY_TAG_RENDERER);

rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platState, backend);
backend->frameNumber = 0;

if(!backend->initialize(backend, applicationName, platState)){

        KFATAL("Renderer backend failed to initialize. Shutting down!");
        return false;
}
return true;
};
static inline void rendererShutdown(){

    backend->shutdown(backend);
    kfree(backend, sizeof(rendererBackend), MEMORY_TAG_RENDERER);

};

static inline b8 rendererBeginFrame(f32 deltaTime) {
    return backend->beginFrame(backend, deltaTime);
}
static inline void rendererOnResized(u16 width, u16 height){};

static inline b8 rendererEndFrame(f32 deltaTime) {
    b8 result = backend->endFrame(backend, deltaTime);
    backend->frameNumber++;
    return result;
}

static inline b8 rendererDrawFrame(renderPacket * packet){

    if(rendererBeginFrame(packet->deltaTime)){
        b8 result = rendererEndFrame(packet->deltaTime);

        if(!result){

            KERROR("RendererEndFrame failed. Application shutting down...");
            return false;
        }
    }
    return true;
};
