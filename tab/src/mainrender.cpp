#include "editors/ElementEditor.hpp"
#include "editors/SceneEditor.hpp"
#include "editors/NodeManager.hpp"
#include "Render.hpp"
#include "utils/JsonUtils.hpp"
#include "raylib.h"

int main() {
    InitWindow(1000, 600, "Novel Renderer");
    SetTargetFPS(60);

    // Initialize data containers
    std::vector<Element> elements;
    std::vector<Scene> scenes;
    std::vector<Node> nodes;

    // Initialize renderer
    Render renderer(elements, scenes, nodes);

    // Load project data
    try {
        JsonUtils::importFromFolder(elements, scenes, nodes, ".");
        TraceLog(LOG_INFO, "Imported project from project.json");

        // Set renderer to the start node
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].isStartNode) {
                renderer.setCurrentNodeIndex(i);
                TraceLog(LOG_INFO, "Set render node to start node %d", i);
                break;
            }
        }
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "Import failed: %s", e.what());
        // Optionally, close the window if import fails
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose()) {
        // Update renderer
        renderer.update(GetTime(), renderer.getCurrentSlide());

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        renderer.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
