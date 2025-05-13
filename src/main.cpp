#include "ElementEditor.hpp"
#include "SceneEditor.hpp"
#include "raylib.h"
#include <string>

enum class Mode { ELEMENT, SCENE };

int main() {
    InitWindow(1000, 600, "Novel Scene Creator");
    SetTargetFPS(60);

    Mode currentMode = Mode::ELEMENT;
    ElementEditor elementEditor;
    SceneEditor sceneEditor(elementEditor.getElements(), elementEditor.getScenes());

    while (!WindowShouldClose()) {
        // Handle mode switching
        if (IsKeyPressed(KEY_TAB)) {
            currentMode = (currentMode == Mode::ELEMENT) ? Mode::SCENE : Mode::ELEMENT;
            TraceLog(LOG_INFO, "Switched to %s mode", currentMode == Mode::ELEMENT ? "Element" : "Scene");
        }

        // Update
        if (currentMode == Mode::ELEMENT) {
            elementEditor.update();
        } else {
            sceneEditor.update();
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (currentMode == Mode::ELEMENT) {
            elementEditor.draw();
        } else {
            sceneEditor.draw();
        }
        DrawText("Press TAB to switch between Element and Scene modes", 10, 560, 10, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
