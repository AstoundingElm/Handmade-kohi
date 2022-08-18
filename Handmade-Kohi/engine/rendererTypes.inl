#pragma once

#include "../defines.h"
#include "maths/mathTypes.h"

enum rendererBackendTypes{

    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
    
};

struct global_uniform_object{

    mat4 projection;  //64 bytes
    mat4 view;       //64 bytes;
    mat4 m_reserved0;
    mat4 m_reserved1;

};

struct renderer_backend {
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);

    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void(*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);    
};

struct renderPacket {
    f32 deltaTime;
};
