#include "../engine/application.cpp"
#include "../engine/kmemory.h"
#include "../engine/input.cpp"
#include "../engine/buttonTypes.h"
struct gameState{
    f32 delaTime;
   
};

static b8 gameInitialize(game * gameInst){
    KDEBUG("gameInitialize() callsed!");
    return true;}

static b8 gameUpdate(game * gameInst, f32 deltaTime){
 static u64 alloc_count = 0;
   u64 prev_alloc_count =  alloc_count;
   alloc_count = get_memory_alloc_count();
  
    if(inputIsKeyUp(KEY_M) && inputWasKeyDown(KEY_M)){
        KDEBUG(" ALlocations: %llu,(%llu this frame) ", alloc_count, alloc_count  - prev_alloc_count);

    }
    
    
    return true;
    
}
static b8 gameRender(game * gameInst, f32 deltaTime){return true;}
static void gameOnResize(game * gameInst, u32 width, u32 height){}
// Define the function to create a game
static b8 create_game(game* out_game) {
    // Application configuration.
    out_game->appConfig.startPosX = 100;
    out_game->appConfig.startPosY = 100;
    out_game->appConfig.startWidth = 1280;
    out_game->appConfig.startHeight = 720;
    //const char * name = "Kohi Engine Testbed";
    out_game->appConfig.name = "fff";
    out_game->update = gameUpdate;
    out_game->render = gameRender; 
    out_game->initialize = gameInitialize;
    out_game->onResize = gameOnResize;
 
    // Create the game state.
    out_game->state = kallocate(sizeof(gameState), MEMORY_TAG_GAME);
    out_game->application_state = 0;
    return true;
}
