
#include "../defines.h"

 internal char * getMemoryUsageStr();
 
#include "clock.inl"
#include "input.h"
#include "rendererFrontend.h"
#include "maths/kmath.h"
#include "linearAllocator.h"

struct applicationConfig{

    i16 startPosX;
      i16 startPosY;
      i16 startWidth;
      i16 startHeight;
      const char * name;

};

 struct game{

applicationConfig appConfig;

b8 (*initialize)(game * gameInst);
b8 (*update)(game * gameInst, f32 deltaTime);
b8 (*render)(game * gameInst, f32 deltaTime);
void (*onResize)(game * gameInst, u32 width, u32 height);

void * state;
void * application_state;

};

struct applicationState{
game * game_inst;
    b8 isRunning;
    b8 is_suspended;
    platform_state platform;
    u32 width;
    u32 height;
    kclock clock;
    f64 lastTime;

    linear_allocator systems_allocator;

    u64 event_system_memory_requirement;
    void* event_system_state;

    u64 memory_system_memory_requirement;
    void* memory_system_state;

    u64 logging_system_memory_requirement;
    void* logging_system_state;

    u64 input_system_memory_requirement;
    void* input_system_state;

    u64 platform_system_memory_requirement;
    void* platform_system_state;

    u64 renderer_system_memory_requirement;
    void* application_renderer_system_state;
};

global_variable  applicationState * app_state;

KINLINE void applicationGetFramebufferSize(u32 * width, u32 * height){

    *width = app_state->width;
    *height  = app_state->height;
}

KINLINE b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context);
KINLINE  b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context);
KINLINE  b8 applicationOnResized(u16 code, void * sender, void * listenerInst, eventContext context);

KINLINE b8 applicationOnResized(u16 code, void * sender, void * listenerInst, eventContext context){

  if (code == EVENT_CODE_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;

            KDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                KINFO("Window minimized, suspending application.");
                app_state->is_suspended = true;
                return true;
            } else {
                if (app_state->is_suspended) {
                    KINFO("Window restored, resuming application.");
                    app_state->is_suspended = false;
                }
                app_state->game_inst->onResize(app_state->game_inst, width, height);
                renderer_on_resized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}



KINLINE b8 applicationCreate(game * game_inst){
   if (game_inst->application_state) {
        KERROR("application_create called more than once.");
        return false;
    }

    game_inst->application_state = kallocate(sizeof(applicationState), MEMORY_TAG_APPLICATION);
    app_state = (applicationState *)game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->isRunning = false;
    app_state->is_suspended = false;

    u64 systems_allocator_total_size = 64 * 1024 * 1024;  // 64 mb
    linear_allocator_create(systems_allocator_total_size, 0, &app_state->systems_allocator);

    // Initialize subsystems.

    // Events
    event_system_initialize(&app_state->event_system_memory_requirement, 0);
    app_state->event_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->event_system_memory_requirement);
    event_system_initialize(&app_state->event_system_memory_requirement, app_state->event_system_state);

    // Memory
    memory_system_initialize(&app_state->memory_system_memory_requirement, 0);
    app_state->memory_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->memory_system_memory_requirement);
    memory_system_initialize(&app_state->memory_system_memory_requirement, app_state->memory_system_state);

    // Logging
    initializeLogging(&app_state->logging_system_memory_requirement, 0);
    app_state->logging_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->logging_system_memory_requirement);
    if (!initializeLogging(&app_state->logging_system_memory_requirement, app_state->logging_system_state)) {
        KERROR("Failed to initialize logging system; shutting down.");
        return false;
    }

    // Input
    input_system_initialize(&app_state->input_system_memory_requirement, 0);
    app_state->input_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->input_system_memory_requirement);
    input_system_initialize(&app_state->input_system_memory_requirement, app_state->input_system_state);

    // Register for engine-level events.
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    event_register(EVENT_CODE_RESIZED, 0, applicationOnResized);

    // Platform
    platform_system_startup(&app_state->platform_system_memory_requirement, 0, 0, 0, 0, 0, 0);
    app_state->platform_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->platform_system_memory_requirement);
    if (!platform_system_startup(
            &app_state->platform_system_memory_requirement,
            app_state->platform_system_state,
            game_inst->appConfig.name,
            game_inst->appConfig.startPosX,
            game_inst->appConfig.startPosY,
            game_inst->appConfig.startWidth,
            game_inst->appConfig.startHeight)) {
        return false;
    }

    // Renderer system
    renderer_system_initialize(&app_state->renderer_system_memory_requirement, 0, 0);
    app_state->application_renderer_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->renderer_system_memory_requirement);
    if (!renderer_system_initialize(&app_state->renderer_system_memory_requirement, app_state->application_renderer_system_state, game_inst->appConfig.name)) {
        KFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    // Initialize the game.
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        KFATAL("Game failed to initialize.");
        return false;
    }

    // Call resize once to ensure the proper size has been set.
    app_state->game_inst->onResize(app_state->game_inst, app_state->width, app_state->height);

    return true;
}
KINLINE KAPI b8 applicationRun(){

app_state->isRunning = true;
 clockStart(&app_state->clock);
    clockUpdate(&app_state->clock);
    app_state->lastTime = app_state->clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    KINFO(getMemoryUsageStr());

    while (app_state->isRunning) {
        if (!platform_pump_messages()) {
            app_state->isRunning = false;
        }

        if (!app_state->is_suspended) {
            // Update clock and get delta time.
            clockUpdate(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta = (current_time - app_state->lastTime);
            f64 frame_start_time = platformGetAbsoluteTime();

            if (!app_state->game_inst->update(app_state->game_inst, (f32)delta)) {
                KFATAL("Game update failed, shutting down.");
                app_state->isRunning = false;
                break;
            }

            // Call the game's render routine.
            if (!app_state->game_inst->render(app_state->game_inst, (f32)delta)) {
                KFATAL("Game render failed, shutting down.");
                app_state->isRunning = false;
                break;
            }

            // TODO: refactor packet creation
            renderPacket packet;
            packet.deltaTime = delta;
            renderer_draw_frame(&packet);

            // Figure out how long the frame took and, if below
            f64 frame_end_time = platformGetAbsoluteTime();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);

                // If there is time left, give it back to the OS.
                b8 limit_frames = false;
                if (remaining_ms > 0 && limit_frames) {
                    platformSleep(remaining_ms - 1);
                }

                frame_count++;
            }

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            input_update(delta);

            // Update last time
            app_state->lastTime = current_time;
        }
    }

   app_state->isRunning = false;

    // Shutdown event system.
    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    input_system_shutdown(app_state->input_system_state);

    renderer_system_shutdown(app_state->application_renderer_system_state);

    platform_system_shutdown(app_state->platform_system_state);

    memory_system_shutdown(app_state->memory_system_state);

    //event_system_shutdown(app_state->event_system_state);
    
    return true;
};



internal b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context) {
 switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            app_state->isRunning = false;
            return true;
        }
    }

    return false;
}

KINLINE b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            eventContext data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this.
            return true;
        } else if (key_code == KEY_A) {
            // Example on checking for a key
            KDEBUG("Explicit - A key pressed!");
        } else {
            KDEBUG("'%c' key pressed in window.", key_code);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_B) {
            // Example on checking for a key
            KDEBUG("Explicit - B key released!");
        } else {
            KDEBUG("'%c' key released in window.", key_code);
        }
    }
    return false;
}
