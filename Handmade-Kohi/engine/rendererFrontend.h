#pragma once
#include "rendererTypes.inl"
#include "rendererBackend.h"
#include "logger.h"
#include "kmemory.h"
#include "maths/kmath.h"

struct renderer_system_state {
    renderer_backend backend;
};

global_variable renderer_system_state* renderer_state_ptr;

KINLINE b8 renderer_system_initialize(u64* memory_requirement, void* state, const char* application_name) {
    *memory_requirement = sizeof(renderer_system_state);
    if (state == 0) {
        return true;
    }
    renderer_state_ptr = (renderer_system_state *)state;

    // TODO: make this configurable.
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &renderer_state_ptr->backend);
    renderer_state_ptr->backend.frame_number = 0;

    if (!renderer_state_ptr->backend.initialize(&renderer_state_ptr->backend, application_name)) {
        KFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

KINLINE void renderer_system_shutdown(void* state) {
    if (renderer_state_ptr) {
        renderer_state_ptr->backend.shutdown(&renderer_state_ptr->backend);
    }
    renderer_state_ptr = 0;
}

KINLINE b8 renderer_begin_frame(f32 delta_time) {
    if (!renderer_state_ptr) {
        return false;
    }
    return renderer_state_ptr->backend.begin_frame(&renderer_state_ptr->backend, delta_time);
}

KINLINE void renderer_on_resized(u16 width, u16 height) {
    if (renderer_state_ptr) {
        renderer_state_ptr->backend.resized(&renderer_state_ptr->backend, width, height);
    } else {
        KWARN("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}


KINLINE b8 renderer_end_frame(f32 delta_time) {
    if (!renderer_state_ptr) {
        return false;
    }
    b8 result = renderer_state_ptr->backend.end_frame(&renderer_state_ptr->backend, delta_time);
    renderer_state_ptr->backend.frame_number++;
    return result;
}

KINLINE b8 renderer_draw_frame(renderPacket* packet) {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (renderer_begin_frame(packet->deltaTime)) {

        mat4 projection = mat4_perspective(deg_to_rad(45.0f), 1280/720.0f, 0.1f, 1000.0f); 
        static f32 z = -1.0f;
        z -= 0.01f;
        mat4 view = mat4_translation((vec3){0, 0, -1.0f});
        renderer_state_ptr->backend.update_global_state(projection, view,vec3_zero(),  vec4_one(), 0);
        
        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = renderer_end_frame(packet->deltaTime);

        if (!result) {
            KERROR("renderer_end_frame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}
