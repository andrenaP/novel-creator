#include "ElementEditor.hpp"
#include "SceneEditor.hpp"
#include "NodeManager.hpp"
#include "Render.hpp"
#include "JsonUtils.hpp"
#include "raylib.h"

#include "raygui.h"

int main() {
    InitWindow(1000, 600, "Novel Renderer");
    SetTargetFPS(60);


    // Define Cyrillic Unicode range (U+0400 to U+04FF)
    int codepoints[512]; // Increased size to include basic Latin + Cyrillic
    int count = 0;
    // Include basic Latin (U+0020 to U+007F) for standard characters
    for (int i = 0x0020; i <= 0x007F; ++i) {
        codepoints[count++] = i;
    }
    // Include Cyrillic (U+0400 to U+04FF)
    for (int i = 0x0400; i <= 0x04FF; ++i) {
        codepoints[count++] = i;
    }

    // Load Noto Sans font
    Font customFont = LoadFontEx("font/noto-sans.regular.ttf", 16, codepoints, count);
    // Alternative: If using variable font, comment the above and uncomment below
    // Font customFont = LoadFontEx("font/NotoSans-VariableFont_wdth,wght.ttf", 16, codepoints, count);
    if (customFont.baseSize == 0 || customFont.glyphCount == 0) {
        TraceLog(LOG_ERROR, "Failed to load font: font/NotoSans-Regular.ttf");
        customFont = GetFontDefault(); // Fallback to default font
        TraceLog(LOG_WARNING, "Using default font as fallback");
    } else {
        TraceLog(LOG_INFO, "Loaded font with %d glyphs", customFont.glyphCount);
    }

    // Set the custom font globally for raygui
    GuiSetFont(customFont);

    // Set global text size for raygui controls
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

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
        renderer.draw(customFont);
        EndDrawing();
    }

    UnloadFont(customFont);
    CloseWindow();
    return 0;
}
