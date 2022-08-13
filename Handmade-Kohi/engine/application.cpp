#include "/home/petermiller/Desktop/Handmade-Kohi/defines.h"

//extern "C"
 static char * getMemoryUsageStr();
#include "clock.inl"
#include "input.cpp"
#include "rendererFrontend.cpp"
#include "kmath.h"

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

};

struct applicationState{
game * gameInst;
    b8 isRunning;
    b8 is_suspended;
    platformState platform;
    i16 width;
    i16 height;
    kclock clock;
    f64 lastTime;
};
static b8 initialized = false;
static applicationState app_state;

 static void applicationGetFramebufferSize(u32 * width, u32 * height){

    *width = app_state.width;
    *height  = app_state.height;
}
static b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context);
static b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context);
static b8 applicationOnResized(u16 code, void * sender, void * listenerInst, eventContext context);

b8 applicationOnResized(u16 code, void * sender, void * listenerInst, eventContext context){

  if (code == EVENT_CODE_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != app_state.width || height != app_state.height) {
            app_state.width = width;
            app_state.height = height;

            KDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                KINFO("Window minimized, suspending application.");
                app_state.is_suspended = true;
                return true;
            } else {
                if (app_state.is_suspended) {
                    KINFO("Window restored, resuming application.");
                    app_state.is_suspended = false;
                }
                app_state.gameInst->onResize(app_state.gameInst, width, height);
                rendererOnResized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}



KINLINE b8 applicationCreate(game * game_inst){
 if (initialized) {
        KERROR("application_create called more than once.");
        return false;
    }

    app_state.gameInst = game_inst;

    // Initialize subsystems.
    initializeLogging();
    inputInitialize();

    // TODO: Remove this
    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);
    KDEBUG("A test message: %f", 3.14f);
    KTRACE("A test message: %f", 3.14f);
    KINFO("YEHAHAHAHAHA");

    app_state.isRunning = true;
    app_state.is_suspended = false;

    if (!eventInitialize()) {
        KERROR("Event system failed initialization. Application cannot continue.");
        return false;
    }

    eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    eventRegister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    eventRegister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    eventRegister(EVENT_CODE_RESIZED, 0, applicationOnResized);

    if (!platformStartup(
            &app_state.platform,
            game_inst->appConfig.name,
            game_inst->appConfig.startPosX,
            game_inst->appConfig.startPosY,
            game_inst->appConfig.startWidth,
            game_inst->appConfig.startHeight)) {
        return false;
    }

    // Renderer startup
    if (!rendererInitialize(game_inst->appConfig.name, &app_state.platform)) {
        KFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    // Initialize the game.
    if (!app_state.gameInst->initialize(app_state.gameInst)) {
        KFATAL("Game failed to initialize.");
        return false;
    }

    app_state.gameInst->onResize(app_state.gameInst, app_state.width, app_state.height);

    initialized = true;

    return true;
          
}
KINLINE KAPI b8 applicationRun(){

 clockStart(&app_state.clock);
    clockUpdate(&app_state.clock);
    app_state.lastTime = app_state.clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    KINFO(getMemoryUsageStr());

    while (app_state.isRunning) {
        if (!platformPumpMessages(&app_state.platform)) {
            app_state.isRunning = false;
        }

        if (!app_state.is_suspended) {
            // Update clock and get delta time.
            clockUpdate(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta = (current_time - app_state.lastTime);
            f64 frame_start_time = platformGetAbsoluteTime();

            if (!app_state.gameInst->update(app_state.gameInst, (f32)delta)) {
                KFATAL("Game update failed, shutting down.");
                app_state.isRunning = false;
                break;
            }

            // Call the game's render routine.
            if (!app_state.gameInst->render(app_state.gameInst, (f32)delta)) {
                KFATAL("Game render failed, shutting down.");
                app_state.isRunning = false;
                break;
            }

            // TODO: refactor packet creation
            renderPacket packet;
            packet.deltaTime = delta;
            rendererDrawFrame(&packet);

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
            inputUpdate(delta);

            // Update last time
            app_state.lastTime = current_time;
        }
    }

    app_state.isRunning = false;// ShUdown event system.
    eventUnRegister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    eventUnRegister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    eventUnRegister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    eventShutdown();
    inputShutdown();

    rendererShutdown();

    platformShutdown(&app_state.platform);

    return true;
};



static b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context) {
 switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            app_state.isRunning = false;
            return true;
        }
    }

    return false;
}

static b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            eventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

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
