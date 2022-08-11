#include "/home/petermiller/Desktop/Handmade-Kohi/testbed/entry.cpp"

extern b8 create_game(game* out_game);

int main(void) {

    initializeMemory();
    // Request the game instance from the application.
    game game_inst;
    if (!create_game(&game_inst)) {
        KFATAL("Could not create game!");
        return -1;
    }

    // Ensure the function pointers exist.
    if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.onResize) {
        KFATAL("The game's function pointers must be assigned!");
        return -2;
    }

    // Initialization.
    if (!applicationCreate(&game_inst)) {
        KINFO("Application failed to create!.");
        return 1;
    }

    // Begin the game loop.
    if(!applicationRun()) {
        KINFO("Application did not shutdown gracefully.");
        return 2;
    }
    shutDownMemory();

    return 0;
}
