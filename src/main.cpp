#include "ElementEditor.hpp"
#include "SceneEditor.hpp"
#include "NodeManager.hpp"
#include "Render.hpp"
#include "JsonUtils.hpp"
#include "raylib.h"

enum class Mode { ELEMENT, SCENE, NODE, RENDER, IMPORT_EXPORT };

class ImportExportManager {
private:
    std::vector<Element>& elements;
    std::vector<Scene>& scenes;
    std::vector<Node>& nodes;
    Render& renderer; // Reference to renderer to update current node after import

public:
    ImportExportManager(std::vector<Element>& elements, std::vector<Scene>& scenes, std::vector<Node>& nodes, Render& renderer)
        : elements(elements), scenes(scenes), nodes(nodes), renderer(renderer) {}

    void update() {
        // No update logic needed for now, but can be extended for dynamic UI
    }

    void draw() {
        // Draw import/export buttons
        if (GuiButton({400, 250, 100, 30}, "Export Project")) {
            try {
                JsonUtils::exportToFile(elements, scenes, nodes, "project.json");
                TraceLog(LOG_INFO, "Exported project to project.json");
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Export failed: %s", e.what());
            }
        }
        if (GuiButton({400, 290, 100, 30}, "Import Project")) {
            try {
                JsonUtils::importFromFile(elements, scenes, nodes, "project.json");
                // Reset renderer to start node after import
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (nodes[i].isStartNode) {
                        renderer.setCurrentNodeIndex(i);
                        TraceLog(LOG_INFO, "Set render node to start node %d after import", i);
                        break;
                    }
                }
                TraceLog(LOG_INFO, "Imported project from project.json");
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Import failed: %s", e.what());
            }
        }
        DrawText("Import/Export Mode\nPress TAB to switch modes", 350, 350, 10, DARKGRAY);
    }
};

int main() {
    InitWindow(1000, 600, "Novel Scene Creator");
    SetTargetFPS(60);

    Mode currentMode = Mode::ELEMENT;
    ElementEditor elementEditor;
    SceneEditor sceneEditor(elementEditor.getElements(), elementEditor.getScenes());
    NodeManager nodeManager(sceneEditor.getScenes());
    Render renderer(elementEditor.getElements(), elementEditor.getScenes(), nodeManager.getNodes());
    ImportExportManager importExportManager(
        elementEditor.getElements(),
        elementEditor.getScenes(),
        nodeManager.getNodes(),
        renderer
    );

    // Set initial node to the start node
    for (size_t i = 0; i < nodeManager.getNodes().size(); ++i) {
        if (nodeManager.getNodes()[i].isStartNode) {
            renderer.setCurrentNodeIndex(i);
            TraceLog(LOG_INFO, "Set initial render node to start node %d", i);
            break;
        }
    }

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
                currentMode = Mode::RENDER;
                renderer.resetSlide();
                TraceLog(LOG_INFO, "Switched to Render mode");
                break;
            case Mode::RENDER:
                currentMode = Mode::IMPORT_EXPORT;
                TraceLog(LOG_INFO, "Switched to Import/Export mode");
                break;
            case Mode::IMPORT_EXPORT:
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
        case Mode::ELEMENT:
            elementEditor.update();
            break;
        case Mode::SCENE:
            sceneEditor.update();
            break;
        case Mode::NODE:
            nodeManager.update();
            break;
        case Mode::RENDER:
            renderer.update(GetTime(), renderer.getCurrentSlide());
            break;
        case Mode::IMPORT_EXPORT:
            importExportManager.update();
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
        case Mode::RENDER:
            renderer.draw();
            break;
        case Mode::IMPORT_EXPORT:
            importExportManager.draw();
            break;
        default:
            break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// #include <raylib.h>
// #include "TabManager.hpp"

// int main() {
//     const int screenWidth = 800;
//     const int screenHeight = 600;

//     InitWindow(screenWidth, screenHeight, "Browser-like Scene Editor");
//     SetTargetFPS(60);

//     // Create tab manager
//     TabManager tabManager(10.0f, 10.0f, 150.0f, 30.0f);

//     // Add initial tab
//     tabManager.addTab("Home");

//     while (!WindowShouldClose()) {
//         // Update
//         tabManager.update();

//         // Draw
//         BeginDrawing();
//         ClearBackground(RAYWHITE);

//         tabManager.draw();

//         // Draw content area


//         // Draw content based on selected tab
//         int selectedTab = tabManager.getSelectedTabIndex();
//         if (selectedTab >= 0) {
//             DrawText(("Content for Tab " + std::to_string(selectedTab + 1)).c_str(),
//                     100, 100, 20, BLACK);
//         }

//         EndDrawing();
//     }

//     CloseWindow();
//     return 0;
// }
