//#include "gameTypes.h"
//#include "entry.h"
//#include "platform.cpp"
#include "application.cpp"
#include "kmemory.h"
struct gameState{
    f32 delaTime;
};

b8 gameInitialize(game * gameInst){
    KDEBUG("gameInitialize() callsed!");
    return true;}

b8 gameUpdate(game * gameInst, f32 deltaTime){return true;}
b8 gameRender(game * gameInst, f32 deltaTime){return true;}
b8 gameOnResize(game * gameInst, u32 width, u32 height){return true;}
// Define the function to create a game
b8 create_game(game* out_game) {
    // Application configuration.
    out_game->appConfig.startPosX = 100;
    out_game->appConfig.startPosY = 100;
    out_game->appConfig.startWidth = 1280;
    out_game->appConfig.startHeight = 720;
    const char * name = "Kohi Engine Testbed";
    out_game->appConfig.name = name;
    out_game->update = gameUpdate;
    out_game->render = gameRender; 
    out_game->initialize = gameInitialize;
    out_game->onResize = gameOnResize;
 
    // Create the game state.
    out_game->state = kallocate(sizeof(gameState), MEMORY_TAG_GAME);
    return TRUE;
}