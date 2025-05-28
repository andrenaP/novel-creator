#include <raylib.h>
#include "managers/TabManager.hpp"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Browser-like Scene Editor");
    SetTargetFPS(60);

    // Создаём TabManager, управляющий вкладками
    TabManager tabManager({10.0f, 10.0f}, screenWidth - 20.0f, screenHeight - 20.0f);

    while (!WindowShouldClose()) 
    {
        // Обработка ввода
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            Vector2 mousePos = GetMousePosition();
            tabManager.handleClick(mousePos);
        }

        // Отрисовка
        BeginDrawing();
        ClearBackground(RAYWHITE);

        tabManager.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
