#include "kmath.h"

#include "platform.h"

#include <math.h>
#include <stdlib.h>

internal b8 rand_seeded = false;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
KINLINE f32 ksin(f32 x) {
    return sinf(x);
}

KINLINE f32 kcos(f32 x) {
    return cosf(x);
}



KINLINE f32 kacos(f32 x) {
    return acosf(x);
}

KINLINE f32 ksqrt(f32 x) {
    return sqrtf(x);
}

KINLINE f32 kabs(f32 x) {
    return fabsf(x);
}

KINLINE i32 krandom() {
    if (!rand_seeded) {
        srand((u32)platformGetAbsoluteTime());
        rand_seeded = true;
    }
    return rand();
}

KINLINE i32 krandom_in_range(i32 min, i32 max) {
    if (!rand_seeded) {
        srand((u32)platformGetAbsoluteTime());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

KINLINE f32 fkrandom() {
    return (float)krandom() / (f32)RAND_MAX;
}

KINLINE f32 fkrandom_in_range(f32 min, f32 max) {
    return min + ((float)krandom() / ((f32)RAND_MAX / (max - min)));
}