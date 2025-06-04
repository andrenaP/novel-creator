#include "ElementEditor.hpp"
#include "SceneEditor.hpp"
#include "NodeManager.hpp"
#include "Render.hpp"
#include "JsonUtils.hpp"
#include "raylib.h"

// #define RAYGUI_IMPLEMENTATION
#include "raygui.h"

enum class Mode { ELEMENT, SCENE, NODE, RENDER, IMPORT_EXPORT };

class ImportExportManager {
private:
    std::vector<Element>& elements;
    std::vector<Scene>& scenes;
    std::vector<Node>& nodes;
    Render& renderer;

public:
    ImportExportManager(std::vector<Element>& elements, std::vector<Scene>& scenes, std::vector<Node>& nodes, Render& renderer)
        : elements(elements), scenes(scenes), nodes(nodes), renderer(renderer) {}

    void update() {
        // No update logic needed for now
    }

    void draw(Font customFont) {
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

        if (GuiButton({400, 330, 100, 30}, "export To Folder")) {
            try {
                JsonUtils::exportToFolder(elements, scenes, nodes, "path");
                TraceLog(LOG_INFO, "Exported project to path");
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Export failed: %s", e.what());
            }
        }

        if (GuiButton({400, 370, 100, 30}, "Import To Folder")) {
            try {
                JsonUtils::importFromFolder(elements, scenes, nodes, "path");
                // Reset renderer to start node after import
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (nodes[i].isStartNode) {
                        renderer.setCurrentNodeIndex(i);
                        TraceLog(LOG_INFO, "Set render node to start node %d after import", i);
                        break;
                    }
                }
                TraceLog(LOG_INFO, "Imported project from path");
            } catch (const std::exception& e) {
                TraceLog(LOG_ERROR, "Import failed: %s", e.what());
            }
        }


        DrawText("Import/Export Mode\nPress TAB to switch modes", 350, 400, 10, DARKGRAY);
    }
};

int main() {
    InitWindow(1000, 600, "Novel Scene Creator");
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
    nodeManager.customFont=customFont;
    // Set initial node to the start node
    for (size_t i = 0; i < nodeManager.getNodes().size(); ++i) {
        if (nodeManager.getNodes()[i].isStartNode) {
            renderer.setCurrentNodeIndex(i);
            TraceLog(LOG_INFO, "Set initial render node to start node %d", i);
            break;
        }
    }

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_TAB)) {
            switch (currentMode) {
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
            }
        }

        switch (currentMode) {
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
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        switch (currentMode) {
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
            renderer.draw(customFont);
            break;
        case Mode::IMPORT_EXPORT:
            importExportManager.draw(customFont);
            break;
        }
        EndDrawing();
    }

    UnloadFont(customFont);
    CloseWindow();
    return 0;
}
