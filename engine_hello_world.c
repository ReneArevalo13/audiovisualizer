/*
   Example to demonstrate how to initialize an audio engine and play a sound.
   This plays a sound specified on the command line.
*/

#include "miniaudio.h"
#include <stdio.h>

int main(int argc, char** argv) {
    ma_result result;
    ma_engine engine;

    if (argc < 2) {
        printf("No input file\n");
        return -1;
    }
    
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine");
        return -1;
    }

    ma_engine_play_sound(&engine, argv[1], NULL);
    printf("Press Enter to quit...");
    getchar();
    ma_engine_uninit(&engine);
    return 0;

}
