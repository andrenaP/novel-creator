#include "ElementEditor.hpp"
#include "SceneEditor.hpp"
#include "NodeManager.hpp"
#include "raylib.h"

enum class Mode { ELEMENT, SCENE, NODE };

int main() {
    InitWindow(1000, 600, "Novel Scene Creator");
    SetTargetFPS(60);

    Mode currentMode = Mode::ELEMENT;
    ElementEditor elementEditor;
    SceneEditor sceneEditor(elementEditor.getElements(), elementEditor.getScenes());
    NodeManager nodeManager(sceneEditor.getScenes());

    while (!WindowShouldClose()) {
        // Handle mode switching
        if (IsKeyPressed(KEY_TAB)) {
            if (currentMode == Mode::ELEMENT) {
                currentMode = Mode::SCENE;
                TraceLog(LOG_INFO, "Switched to Scene mode");
            } else if (currentMode == Mode::SCENE) {
                currentMode = Mode::NODE;
                TraceLog(LOG_INFO, "Switched to Node mode");
            } else {
                currentMode = Mode::ELEMENT;
                TraceLog(LOG_INFO, "Switched to Element mode");
            }
        }

        // Update
        if (currentMode == Mode::ELEMENT) {
            elementEditor.update();
        } else if (currentMode == Mode::SCENE) {
            sceneEditor.update();
        } else {
            nodeManager.update();
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (currentMode == Mode::ELEMENT) {
            elementEditor.draw();
        } else if (currentMode == Mode::SCENE) {
            sceneEditor.draw();
        } else {
            nodeManager.draw();
        }
        DrawText("Press TAB to switch between Element, Scene, and Node modes", 10, 560, 10, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
