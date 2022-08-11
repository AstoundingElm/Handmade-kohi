#pragma once

#include "/home/petermiller/Desktop/Handmade-Kohi/defines.h"

#include "/home/petermiller/Desktop/Handmade-Kohi/engine/darray.h"
#define MAX_MESSAGE_CODES 16384

struct eventContext{

    union {
  i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
};


 enum systemEventCode {
    // Shuts the application down on the next frame.
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // Keyboard key pressed.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_PRESSED = 0x02,

    // Keyboard key released.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_RELEASED = 0x03,

    // Mouse button pressed.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    // Mouse button released.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    // Mouse moved.
    /* Context usage:
     * u16 x = data.data.u16[0];
     * u16 y = data.data.u16[1];
     */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    // Mouse moved.
    /* Context usage:
     * u8 z_delta = data.data.u8[0];
     */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    // Resized/resolution changed from the OS.
    /* Context usage:
     * u16 width = data.data.u16[0];
     * u16 height = data.data.u16[1];
     */
    EVENT_CODE_RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
};

typedef b8 (*PFN_on_event)(u16 code, void * sender, void * listenerInst, eventContext data);

 struct registeredEvent{

void * listener;
PFN_on_event callback;

};


struct eventCodeEntry{

    registeredEvent * events;
};

struct eventSystemState{

    eventCodeEntry registered[MAX_MESSAGE_CODES];
};

static b8 isInitialized = false;

static eventSystemState state;

inline static b8 eventInitialize(){
if(isInitialized == true){
    return false;
};
if(isInitialized == false)
kzeroMemory(&state, sizeof(state));
isInitialized = true;
    return true;

}

inline static void eventShutdown(){
for(u16 i = 0; i <MAX_MESSAGE_CODES; i ++){

    if(state.registered[i].events != 0 ){
        darray_destroy(state.registered[i].events);
        state.registered[i].events = 0;
    }
}


}

static inline KAPI b8 eventRegister(u16 code, void * listener, PFN_on_event onEvent){
 if(isInitialized == false) {
        return false;
    }

    if(state.registered[code].events == 0) {
        state.registered[code].events = (registeredEvent *)darrayCreate(registeredEvent);
    }

    u64 registered_count = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        if(state.registered[code].events[i].listener == listener) {
            // TODO: warn
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registeredEvent event;
    event.listener = listener;
    event.callback = onEvent;
    darray_push(state.registered[code].events, event);

    return true;


};

static inline b8 eventUnRegister(u16 code, void * listener, PFN_on_event onEvent){

     if(isInitialized == false) {
        return false;
    }

    // On nothing is registered for the code, boot out.
    if(state.registered[code].events == 0) {
        // TODO: warn
        return false;
    }

    u64 registered_count = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        registeredEvent e = state.registered[code].events[i];
        if(e.listener == listener && e.callback == onEvent) {
            // Found one, remove it
            registeredEvent popped_event;
            darray_pop_at(state.registered[code].events, i, &popped_event);
            return true;
        }
    }

    // Not found.
    return false;
};

static inline KAPI b8 eventFire(u16 code, void * sender, eventContext context){

     if(isInitialized == false) {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if(state.registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; ++i) {
        registeredEvent e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context)) {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}
