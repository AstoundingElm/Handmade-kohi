#pragma once
#include "eventTypes.h"
#include "../defines.h"

#include "darray.h"

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

static b8 is_initialized = false;

static eventSystemState state;

static inline b8 eventInitialize(){
  if (is_initialized == true) {
        return false;
    }
    is_initialized = false;
    kzeroMemory(&state, sizeof(state));

    is_initialized = true;

    return true;

}

KINLINE void eventShutdown(){
for(u16 i = 0; i < MAX_MESSAGE_CODES; ++i){
        if(state.registered[i].events != 0) {
            darray_destroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    };
}




KINLINE KAPI b8 eventRegister(u16 code, void * listener, PFN_on_event onEvent){
     if(is_initialized == false) {
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

KINLINE b8 eventUnRegister(u16 code, void * listener, PFN_on_event onEvent){

   if(is_initialized == false) {
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

 KAPI static b8 eventFire(u16 code, void * sender, eventContext context){

     if(is_initialized == false) {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if(state.registered[code].events == 0) {
        return false;
    }

    u64 registered_count = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registered_count; ++i) {
        registeredEvent e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context)) {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}
