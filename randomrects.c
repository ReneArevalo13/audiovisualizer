#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"

int main() {

    const int screenWidth = 800;
    const int screenHeight = 500;
    const int N = 150;
    const int hh = screenHeight/2; // half height

    InitWindow(screenWidth, screenHeight, "random rectangles");
    SetTargetFPS(60);
    srand(time(NULL));
    

    // generate N random heights between 0-100
    while(!WindowShouldClose()) {
        
        BeginDrawing();
        ClearBackground(WHITE);
        float cell_width = (float)screenWidth/N;
        for (int i=0; i < N; i++) {
            float height = (float)rand() / RAND_MAX; // should be between 0-100

           // printf("%f", height);
            float s_height = (screenHeight/2)*height; // scaled height
           // printf("%f", s_height);
            DrawRectangle(i*cell_width,  hh - s_height , 5, s_height, BLACK);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
