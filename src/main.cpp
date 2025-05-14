// #include "ElementEditor.hpp"
// #include "SceneEditor.hpp"
// #include "NodeManager.hpp"
// #include "raylib.h"

// enum class Mode { ELEMENT, SCENE, NODE };

// int main() {
//     InitWindow(1000, 600, "Novel Scene Creator");
//     SetTargetFPS(60);

//     Mode currentMode = Mode::ELEMENT;
//     ElementEditor elementEditor;
//     SceneEditor sceneEditor(elementEditor.getElements(), elementEditor.getScenes());
//     NodeManager nodeManager(sceneEditor.getScenes());

//     while (!WindowShouldClose()) 
//     {
//         // Handle mode switching
//         if (IsKeyPressed(KEY_TAB)) 
//         {
//             switch (currentMode)
//             { 
//             case Mode::ELEMENT: 
//                 currentMode = Mode::SCENE;
//                 TraceLog(LOG_INFO, "Switched to Scene mode");
//                 break;
//             case Mode::SCENE:
//                 currentMode = Mode::NODE;
//                 TraceLog(LOG_INFO, "Switched to Node mode");
//                 break;
//             case Mode::NODE:
//                 currentMode = Mode::ELEMENT;
//                 TraceLog(LOG_INFO, "Switched to Element mode");
//                 break;
            
//             default:
//                 break;
//             }
//         }

//         // Update
//         switch (currentMode)
//         { 
//         // If you add more than one line to the body block, it is better to wrap it in {}  
//         case Mode::ELEMENT: 
//             elementEditor.update();
//             break;
//         case Mode::SCENE:
//             sceneEditor.update();
//             break;
//         case Mode::NODE:
//             nodeManager.update();
//             break;
        
//         default:
//             break;
//         }

//         // Draw
//         BeginDrawing();
//         ClearBackground(RAYWHITE);
//         switch (currentMode)
//         { 
//         case Mode::ELEMENT: 
//             elementEditor.draw();
//             break;
//         case Mode::SCENE:
//             sceneEditor.draw();
//             break;
//         case Mode::NODE:
//             nodeManager.draw();
//             break;
        
//         default:
//             break;
//         }
//         DrawText("Press TAB to switch between\nElement, Scene, and Node modes", 800, 560, 10, DARKGRAY);
//         EndDrawing();
//     }

//     CloseWindow();
//     return 0;
// }

#include <raylib.h>
#include "TabManager.hpp"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Browser-like Scene Editor");
    SetTargetFPS(60);

    // Create tab manager
    TabManager tabManager(10.0f, 10.0f, 150.0f, 30.0f);

    // Add initial tab
    tabManager.addTab("Home");

    while (!WindowShouldClose()) {
        // Update
        tabManager.update();

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        tabManager.draw();

        // Draw content area
        

        // Draw content based on selected tab
        int selectedTab = tabManager.getSelectedTabIndex();
        if (selectedTab >= 0) {
            DrawText(("Content for Tab " + std::to_string(selectedTab + 1)).c_str(), 
                    100, 100, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}