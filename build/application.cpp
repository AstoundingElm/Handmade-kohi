#include "defines.h"

extern "C" char * getMemoryUsageStr();

#include "input.cpp"

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
b8 (*onResize)(game * gameInst, u32 width, u32 height);

void * state;

};

struct applicationState{
game * gameInst;
    b8 isRunning;
    b8 isSuspended;
    platformState platform;
    i16 width;
    i16 height;
    f64 lastTime;
};

b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context);

static b8 initialized = false;
static applicationState appState;

b8 applicationCreate(game * gameInst){

    if(initialized){
        KERROR("Application create called more than once!");
        return false;
    }
    

    appState.gameInst = gameInst;
    initializeLogging();
   inputInitialize();
      KFATAL("A test message! %f", 3.14f);
      KERROR("A test message! %f", 3.14f);
      
       KWARN("A test message! %f", 3.14f);
        KINFO("A test message! %f", 3.14f);
         KDEBUG("A test message! %f", 3.14f);
          KTRACE("A test message! %f", 3.14f);

          appState.isRunning = true;
          appState.isSuspended = false;
     
     if(!eventInitialize()){

        KERROR("Event system failed to initialize.");
        return FALSE;
     }

     eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    eventRegister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    eventRegister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
          if(!platformStartup(&appState.platform, 
    gameInst->appConfig.name, 
    gameInst->appConfig.startPosX,  
    gameInst->appConfig.startPosY,
        gameInst->appConfig.startWidth, 
        gameInst->appConfig.startHeight)){
            return false;
          }

    if (!appState.gameInst->initialize(appState.gameInst)) {
        KFATAL("Game failed to initialize.");
        return FALSE;
    }

    appState.gameInst->onResize(appState.gameInst, appState.width, appState.height);
          initialized = true;
          return true;
          
}
KAPI b8 applicationRun(){


KINFO(getMemoryUsageStr());
    while(appState.isRunning){

                    if(!platformPumpMessages(&appState.platform)) {
            appState.isRunning = FALSE;
        }

        if(!appState.isSuspended) {
            if (!appState.gameInst->update(appState.gameInst, (f32)0)) {
                KFATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            // Call the game's render routine.
            if (!appState.gameInst->render(appState.gameInst, (f32)0)) {
                KFATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            inputUpdate(0);
        }
    }
            appState.isRunning = false;

            eventUnRegister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
            eventUnRegister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
            eventUnRegister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
            eventShutdown();
            inputShutdown(); 
            platformShutdown(&appState.platform);
            return true;
};


b8 application_on_event(u16 code, void* sender, void* listener_inst, eventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState.isRunning = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, eventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            eventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this.
            return TRUE;
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
    return FALSE;
}