#include <iostream>

#include "raygui.h"
#include "raylib.h"



int main() {
    float screenWidth = 800;
    float screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Поиск кратчайшего пути");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello, World", GetScreenWidth() / 2, GetScreenHeight() / 2, 30, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
