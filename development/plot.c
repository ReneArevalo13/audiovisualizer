#include "raylib.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Axis Labels");

    // --- Define Axis Properties ---
    Vector2 xAxisOrigin = {screenWidth / 2, screenHeight - 50};
    Vector2 yAxisOrigin = {50, screenHeight / 2};
    float xAxisLength = 400;
    float yAxisLength = 300;

    // --- Data for the graph (example) ---
    float dataValues[] = {0.5f, 1.0f, 1.5f, 2.0f, 2.5f};
    int dataCount = sizeof(dataValues) / sizeof(dataValues[0]);

    // --- Set up the graph (example) ---
    float dataScale = 0.5f; // Example: data values are scaled down by 0.5

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // --- Draw the axes ---
        DrawLine(xAxisOrigin.x - xAxisLength / 2, xAxisOrigin.y, xAxisOrigin.x + xAxisLength / 2, xAxisOrigin.y, WHITE);
        DrawLine(yAxisOrigin.x, yAxisOrigin.y - yAxisLength / 2, yAxisOrigin.x, yAxisOrigin.y + yAxisLength / 2, WHITE);

        // --- Draw the graph (example) ---
        for (int i = 0; i < dataCount; i++) {
            Vector2 point = {xAxisOrigin.x - xAxisLength / 2 + i * (xAxisLength / (dataCount - 1)),
                             yAxisOrigin.y - dataValues[i] * dataScale * yAxisLength / 2};
            DrawCircle(point.x, point.y, 5, GREEN);
        }

        // --- Draw labels ---
        char labelText[32]; // Example buffer for the label text

        // --- X-axis labels ---
        for (int i = 0; i < dataCount; i++) {
        //    printf(labelText, "%d", i); // Example: Integer labels
            int labelWidth = MeasureText(labelText, 20);
            int labelX = xAxisOrigin.x - xAxisLength / 2 + i * (xAxisLength / (dataCount - 1)) - labelWidth / 2;
            int labelY = xAxisOrigin.y + 20; // Offset from the axis line
            DrawText(labelText, labelX, labelY, 20, WHITE);
        }

        // --- Y-axis labels ---
        DrawText("Y Axis", 30, 40, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
