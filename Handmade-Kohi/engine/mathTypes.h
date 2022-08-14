#pragma once
 
#include "../defines.h"
#include "kmemory.h"
union vec2{

    f32 elements[2];
    struct{
        union{

            f32 x, r, s, u;
        };

        union {

            f32 y, g, t, v;
        };
    };

};

struct vec3{
    union{

    f32 elements[3];
    struct{

        union{

            f32 x, r , s, u;
        };

        union{

            f32 y, g, t, v;
        };

        union{

            f32 z, b, p, w;
        

        };
    };
};
};

union vec4{
//#if defined(KUSE_SIMD)
//alignas(16) __m128 data;
//#endif
alignas(16) f32 elements[4];
union{
    struct{
        union{
            f32 x, r, s;
        };
        union {f32 y, g, t;
        };
        union{
            f32 z, b, p;
        };
        union{

            f32 w, a, q;
        };

    };
};

};

typedef vec4 quat; 

union mat4 {

    alignas(16) f32 data[16];

    
};
