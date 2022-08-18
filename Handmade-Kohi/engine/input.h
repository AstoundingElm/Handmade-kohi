#pragma once

#include "../defines.h"
#include "event.h"

struct keyboard_state{

    b8 keys[256];
};

struct mouse_state{

    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
};

struct input_state{

    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;

};

global_variable input_state* input_state_ptr;

KINLINE void input_system_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(input_state);
    if (state == 0) {
        return;
    }
    kzeroMemory(state, sizeof(input_state));
    input_state_ptr = (input_state *)state;

    KINFO("Input subsystem initialized.");
}

KINLINE void input_system_shutdown(void* state) {
    // TODO: Add shutdown routines when needed.
    input_state_ptr = 0;
}

KINLINE void input_update(f64 delta_time) {
    if (!input_state_ptr) {
        return;
    }

    // Copy current states to previous states.
    kcopyMemory(&input_state_ptr->keyboard_previous, &input_state_ptr->keyboard_current, sizeof(keyboard_state));
    kcopyMemory(&input_state_ptr->mouse_previous, &input_state_ptr->mouse_current, sizeof(mouse_state));
}

KINLINE KAPI b8 inputIsKeyDown(keys key){
   if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->keyboard_current.keys[key] == true;

}

KINLINE KAPI b8 inputIsKeyUp(keys key){
 if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->keyboard_current.keys[key] == false;
}

KINLINE KAPI b8 inputWasKeyDown(keys key){

 if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->keyboard_previous.keys[key] == true;
};

KINLINE KAPI b8 inputWasKeyUp(keys key){

 if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->keyboard_previous.keys[key] == false;
}

void input_process_key(keys key, b8 pressed){

 if (key == KEY_LALT) {
        KINFO("Left alt pressed.");
    } else if (key == KEY_RALT) {
        KINFO("Right alt pressed.");
    }

    if (key == KEY_LCONTROL) {
        KINFO("Left ctrl pressed.");
    } else if (key == KEY_RCONTROL) {
        KINFO("Right ctrl pressed.");
    }

    if (key == KEY_LSHIFT) {
        KINFO("Left shift pressed.");
    } else if (key == KEY_RSHIFT) {
        KINFO("Right shift pressed.");
    }

    // Only handle this if the state actually changed.
    if (input_state_ptr->keyboard_current.keys[key] != pressed) {
        // Update internal state_ptr->
        input_state_ptr->keyboard_current.keys[key] = pressed;

        // Fire off an event for immediate processing.
        eventContext context;
        context.data.u16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }

}

KINLINE KAPI b8 inputIsButtonDown(buttons button){
   if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->mouse_current.buttons[button] == true;
}

KINLINE KAPI b8 inputIsButtonUp(buttons button){

 if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->mouse_current.buttons[button] == false;

}

b8 input_was_button_down(buttons button) {
    if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->mouse_previous.buttons[button] == true;
}

KINLINE b8 input_was_button_up(buttons button) {
    if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->mouse_previous.buttons[button] == false;
}

KINLINE void input_get_mouse_position(i32* x, i32* y) {
    if (!input_state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = input_state_ptr->mouse_current.x;
    *y = input_state_ptr->mouse_current.y;
}

KINLINE void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!input_state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = input_state_ptr->mouse_previous.x;
    *y = input_state_ptr->mouse_previous.y;
}


void input_process_button(buttons button, b8 pressed) {
    // If the state changed, fire an event.
    if (input_state_ptr->mouse_current.buttons[button] != pressed) {
        input_state_ptr->mouse_current.buttons[button] = pressed;

        // Fire the event.
        eventContext context;
        context.data.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    // Only process if actually different
    if (input_state_ptr->mouse_current.x != x || input_state_ptr->mouse_current.y != y) {
        // NOTE: Enable this if debugging.
        // KDEBUG("Mouse pos: %i, %i!", x, y);

        // Update internal state_ptr->
        input_state_ptr->mouse_current.x = x;
        input_state_ptr->mouse_current.y = y;

        // Fire the event.
        eventContext context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

 void input_process_mouse_wheel(i8 z_delta) {
    // NOTE: no internal state to update.

    // Fire the event.
    eventContext context;
    context.data.u8[0] = z_delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}