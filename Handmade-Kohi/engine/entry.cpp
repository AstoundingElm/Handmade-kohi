#include "/home/petermiller/Desktop/Handmade-Kohi/testbed/testbed.cpp"

  static b8 create_game(game* out_game);



int main(void) {

    initializeMemory();
    // request the game instance from the application.
    game game_inst;
    if (!create_game(&game_inst)) {
        KFATAL("could not create game!");
        return -1;
    }

    // ensure the function pointers exist.
    if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.onResize) {
        KFATAL("the game's function pointers must be assigned!");
        return -2;
    }

    // initialization.
    if (!applicationCreate(&game_inst)) {
        KINFO("application failed to create!.");
        return 1;
    }

    // begin the game loop.
    if(!applicationRun()) {
        KINFO("application did not shutdown gracefully.");
        return 2;
    }
    shutDownMemory();

    return 0;
}
