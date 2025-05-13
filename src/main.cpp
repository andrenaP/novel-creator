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

    while (!WindowShouldClose()) 
    {
        // Handle mode switching
        if (IsKeyPressed(KEY_TAB)) 
        {
            switch (currentMode)
            { 
            case Mode::ELEMENT: 
                currentMode = Mode::SCENE;
                TraceLog(LOG_INFO, "Switched to Scene mode");
                break;
            case Mode::SCENE:
                currentMode = Mode::NODE;
                TraceLog(LOG_INFO, "Switched to Node mode");
                break;
            case Mode::NODE:
                currentMode = Mode::ELEMENT;
                TraceLog(LOG_INFO, "Switched to Element mode");
                break;
            
            default:
                break;
            }
        }

        // Update
        switch (currentMode)
        { 
        // If you add more than one line to the body block, it is better to wrap it in {}  
        case Mode::ELEMENT: 
            elementEditor.update();
            break;
        case Mode::SCENE:
            sceneEditor.update();
            break;
        case Mode::NODE:
            nodeManager.update();
            break;
        
        default:
            break;
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        switch (currentMode)
        { 
        case Mode::ELEMENT: 
            elementEditor.draw();
            break;
        case Mode::SCENE:
            sceneEditor.draw();
            break;
        case Mode::NODE:
            nodeManager.draw();
            break;
        
        default:
            break;
        }
        DrawText("Press TAB to switch between\nElement, Scene, and Node modes", 800, 560, 10, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
