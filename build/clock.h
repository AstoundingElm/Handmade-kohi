#pragma once
#include "platform.h"

struct kclock{

f64 startTime;
f64 elapsed;
};

static inline void clockUpdate(kclock * clock){

    if(clock->startTime != 0){
        clock->elapsed = platformGetAbsoluteTime() - clock->startTime;
    }
};

static inline void clockStart(kclock * clock){

    clock->startTime = platformGetAbsoluteTime();
    clock->elapsed = 0;
};

static inline void clockStop(kclock * clock){

    clock->startTime = 0;
};