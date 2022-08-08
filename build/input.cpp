#pragma once

#include "defines.h"
#include "event.h"
#include "buttonTypes.h"

struct keyBoardState{

    b8 keys[256];
};

struct mouseState{

    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
};

struct inputState{

    keyBoardState keyboardCurrent;
    keyBoardState keyboardPrevious;
    mouseState mouseCurrent;
    mouseState mousePrevious;

};

static b8 inpInitialized = false;
static inputState inpState = {};

static inline void inputInitialize(){
    kzeroMemory(&state, sizeof(inputState));
    inpInitialized = true;
    KINFO("Input subsystem initialized");

};

static inline void inputShutdown(){

    inpInitialized = false;

};

static inline void inputUpdate(f64 deltaTime){

    if(!inpInitialized){

        return;
    }
kcopyMemory(&inpState.keyboardPrevious, &inpState.keyboardCurrent, sizeof(keyBoardState));
kcopyMemory(&inpState.mousePrevious, &inpState.mouseCurrent, sizeof(mouseState));

};

static inline KAPI b8 inputIsKeyDown(keys key){
     if (!inpInitialized) {
        return FALSE;
    }
    return inpState.keyboardCurrent.keys[key] == TRUE;

}

static inline KAPI b8 inputIsKeyUp(keys key){
 if (!inpInitialized) {
        return FALSE;
    }
    return inpState.keyboardCurrent.keys[key] == FALSE;

}

static inline KAPI b8 inputWasKeyDown(keys key){

      if (!inpInitialized) {
        return FALSE;
    }
    return inpState.keyboardPrevious.keys[key] == TRUE;
};

static inline KAPI b8 inputWasKeyUp(keys key){

      if (!inpInitialized) {
        return TRUE;
    }
    return inpState.keyboardPrevious.keys[key] == FALSE;
}

void inputProcessKey(keys key, b8 pressed){

  if (inpState.keyboardCurrent.keys[key] != pressed) {
        // Update internal state.
        inpState.keyboardCurrent.keys[key] = pressed;

        // Fire off an event for immediate processing.
        eventContext context;
        context.data.u16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }

}

static inline KAPI b8 inputIsButtonDown(buttons button){
   if (!inpInitialized) {
        return FALSE;
    }
    return inpState.mouseCurrent.buttons[button] == TRUE;
}

static inline KAPI b8 inputIsButtonUp(buttons button){


   if (!inpInitialized) {
        return TRUE;
    }
    return inpState.mouseCurrent.buttons[button] == FALSE;

}

static inline KAPI b8 inputWasButtonDown(buttons button){

    if (!inpInitialized) {
        return FALSE;
    }
    return inpState.mousePrevious.buttons[button] == TRUE;
};

static inline KAPI b8 inputWasButtonUp(buttons button){

        if (!inpInitialized) {
        return TRUE;
    }
    return inpState.mousePrevious.buttons[button] == FALSE;
}

static inline KAPI void inputGetMousePosition(i32* x, i32* y){

       if (!inpInitialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = inpState.mouseCurrent.x;
    *y = inpState.mouseCurrent.y;
}
KAPI void inputGetPreviousMousePosition(i32* x, i32* y){

     if (!inpInitialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = inpState.mousePrevious.x;
    *y = inpState.mousePrevious.y;
}

 void inputProcessButton(buttons button, b8 pressed){

      // If the state changed, fire an event.
    if (inpState.mouseCurrent.buttons[button] != pressed) {
        inpState.mouseCurrent.buttons[button] = pressed;

        // Fire the event.
        eventContext context;
        context.data.u16[0] = button;
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}
 void inputProcessMouseMove(i16 x, i16 y){

      // Only process if actually different
    if (inpState.mouseCurrent.x != x || inpState.mouseCurrent.y != y) {
        // NOTE: Enable this if debugging.
        //KDEBUG("Mouse pos: %i, %i!", x, y);

        // Update internal state.
        inpState.mouseCurrent.x = x;
        inpState.mouseCurrent.y = y;

        // Fire the event.
        eventContext context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        eventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}
static inline void inputProcessMouseWheel(i8 z_delta){

        // NOTE: no internal state to update.

    // Fire the event.
    eventContext context;
    context.data.u8[0] = z_delta;
    eventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}