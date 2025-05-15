// #include "ElementEditor.hpp"
// #include "SceneEditor.hpp"
// #include "NodeManager.hpp"
// #include "Render.hpp"
// #include "JsonUtils.hpp"
// #include "raylib.h"

// enum class Mode { ELEMENT, SCENE, NODE, RENDER };

// int main() {
//     InitWindow(1000, 600, "Novel Scene Creator");
//     SetTargetFPS(60);

//     Mode currentMode = Mode::ELEMENT;
//     ElementEditor elementEditor;
//     SceneEditor sceneEditor(elementEditor.getElements(), elementEditor.getScenes());
//     NodeManager nodeManager(sceneEditor.getScenes());
//     Render renderer(elementEditor.getElements(), elementEditor.getScenes(), nodeManager.getNodes());

//     // Set initial node to the start node
//     for (size_t i = 0; i < nodeManager.getNodes().size(); ++i) {
//         if (nodeManager.getNodes()[i].isStartNode) {
//             renderer.setCurrentNodeIndex(i);
//             TraceLog(LOG_INFO, "Set initial render node to start node %d", i);
//             break;
//         }
//     }

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
//                 currentMode = Mode::RENDER;
//                 renderer.resetSlide(); // Reset slide when entering RENDER mode
//                 TraceLog(LOG_INFO, "Switched to Render mode");
//                 break;
//             case Mode::RENDER:
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
//         case Mode::ELEMENT:
//             elementEditor.update();
//             break;
//         case Mode::SCENE:
//             sceneEditor.update();
//             break;
//         case Mode::NODE:
//             nodeManager.update();
//             break;
//         case Mode::RENDER:
//             renderer.update(GetTime(), renderer.getCurrentSlide());
//             // Handle slide navigation via buttons (handled in draw for GUI)
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
//         case Mode::RENDER:
//             renderer.draw();
//             DrawText(TextFormat("Slide: %d", renderer.getCurrentSlide()), 10, 30, 10, DARKGRAY);
//             // Draw slide navigation buttons
//             if (GuiButton({10, 50, 100, 30}, "Next Slide") && renderer.canGoNext()) {
//                 renderer.nextSlide();
//             }
//             if (GuiButton({120, 50, 100, 30}, "Previous Slide") && renderer.canGoPrev()) {
//                 renderer.prevSlide();
//             }
//             if (GuiButton({230, 50, 100, 30}, "Reset Slide")) {
//                 renderer.resetSlide();
//             }
//             break;
//         default:
//             break;
//         }

//         // Draw import/export buttons and mode switch text
//         if (GuiButton({800, 460, 100, 30}, "Export Project")) {
//             try {
//                 JsonUtils::exportToFile(
//                     elementEditor.getElements(),
//                     elementEditor.getScenes(),
//                     nodeManager.getNodes(),
//                     "project.json"
//                 );
//                 TraceLog(LOG_INFO, "Exported project to project.json");
//             } catch (const std::exception& e) {
//                 TraceLog(LOG_ERROR, "Export failed: %s", e.what());
//             }
//         }
//         if (GuiButton({800, 500, 100, 30}, "Import Project")) {
//             try {
//                 JsonUtils::importFromFile(
//                     elementEditor.getElements(),
//                     elementEditor.getScenes(),
//                     nodeManager.getNodes(),
//                     "project.json"
//                 );
//                 // Reset renderer to start node after import
//                 for (size_t i = 0; i < nodeManager.getNodes().size(); ++i) {
//                     if (nodeManager.getNodes()[i].isStartNode) {
//                         renderer.setCurrentNodeIndex(i);
//                         TraceLog(LOG_INFO, "Set render node to start node %d after import", i);
//                         break;
//                     }
//                 }
//                 TraceLog(LOG_INFO, "Imported project from project.json");
//             } catch (const std::exception& e) {
//                 TraceLog(LOG_ERROR, "Import failed: %s", e.what());
//             }
//         }
//         DrawText("Press TAB to switch between\nElement, Scene, Node, and Render modes", 800, 540, 10, DARKGRAY);
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
