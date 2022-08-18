#pragma once
#include "platform.h"

struct kclock{

f64 start_time;
f64 elapsed;
};

KINLINE void clockUpdate(kclock* clock) {
    if (clock->start_time != 0) {
        clock->elapsed = platformGetAbsoluteTime() - clock->start_time;
    }
}

KINLINE void clockStart(kclock* clock) {
    clock->start_time = platformGetAbsoluteTime();
    clock->elapsed = 0;
}

KINLINE void clockStop(kclock* clock) {
    clock->start_time = 0;
}