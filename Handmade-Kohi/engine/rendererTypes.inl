#pragma once

#include "../defines.h"

enum rendererBackendTypes{

    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
    
};

struct rendererBackend {
    struct platformState* platState;
    u64 frameNumber;

    b8 (*initialize)(struct rendererBackend* backend, const char* application_name, struct platformState* platState);

    void (*shutdown)(struct rendererBackend* backend);

    void (*resized)(struct rendererBackend* backend, u16 width, u16 height);

    b8 (*beginFrame)(struct rendererBackend* backend, f32 deltaTime);
    b8 (*endFrame)(struct rendererBackend* backend, f32 deltaTime);    
};

struct renderPacket {
    f32 deltaTime;
};
