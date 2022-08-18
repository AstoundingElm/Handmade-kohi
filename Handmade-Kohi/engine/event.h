#pragma once
#include "eventTypes.h"
#include "../defines.h"

#include "containers/darray.h"

typedef b8 (*PFN_on_event)(u16 code, void * sender, void * listenerInst, eventContext data);

 struct registered_event{

void * listener;

PFN_on_event callback;

};

struct eventCodeEntry{

    registered_event * events;
};

struct event_system_state{

    eventCodeEntry registered[MAX_MESSAGE_CODES];
};

global_variable event_system_state* event_state_ptr;

KINLINE void event_system_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(event_system_state);
    if (state == 0) {
        return;
    }
    kzeroMemory(state, sizeof(state));
    event_state_ptr = (event_system_state *)state;
}

KINLINE void event_system_shutdown(void* state) {
     if (event_state_ptr) {
        // Free the events arrays. And objects pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
            if (event_state_ptr->registered[i].events != 0) {
                darray_destroy(event_state_ptr->registered[i].events);
                event_state_ptr->registered[i].events = 0;
            }
        }
        
    }
    event_state_ptr = 0;
}

KINLINE b8 event_register(u16 code, void* listener, PFN_on_event on_event) {
    if (!event_state_ptr) {
        return false;
    }

    if (event_state_ptr->registered[code].events == 0) {
        event_state_ptr->registered[code].events = (registered_event*)darrayCreate(registered_event);
    }

    u64 registered_count = darrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        if (event_state_ptr->registered[code].events[i].listener == listener) {
            // TODO: warn
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    darray_push(event_state_ptr->registered[code].events, event);

    return true;
}

KINLINE b8 event_unregister(u16 code, void* listener, PFN_on_event on_event) {
    if (!event_state_ptr) {
        return false;
    }

    // On nothing is registered for the code, boot out.
    if (event_state_ptr->registered[code].events == 0) {
        // TODO: warn
        return false;   
    }

    u64 registered_count = darrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        registered_event e = event_state_ptr->registered[code].events[i];
        if (e.listener == listener && e.callback == on_event) {
            // Found one, remove it
            registered_event popped_event;
            darray_pop_at(event_state_ptr->registered[code].events, i, &popped_event);
            return true;
        }
    }

    // Not found.
    return false;
}

 b8 event_fire(u16 code, void* sender, eventContext context) {
    if (!event_state_ptr) {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if (event_state_ptr->registered[code].events == 0) {
        return false;
    }

    u64 registered_count = darrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        registered_event e = event_state_ptr->registered[code].events[i];
        if (e.callback(code, sender, e.listener, context)) {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}
