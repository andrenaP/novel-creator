#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "raylib.h"
#include "raygui.h"
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <variant>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;

// Element Types
struct TextElement {
    std::string content;
};

struct CharacterElement {
    std::string name;
    std::vector<std::pair<std::string, std::string>> images; // pair<imageName, imagePath>
    std::vector<Texture2D> textures; // Corresponding textures for images

    CharacterElement() = default;
    ~CharacterElement() {
        for (auto& texture : textures) {
            if (texture.id > 0) UnloadTexture(texture);
        }
    }
};

struct BackgroundElement {
    std::string imagePath;
    Texture2D texture = {0};

    ~BackgroundElement() {
        if (texture.id > 0) UnloadTexture(texture);
    }
};

enum class ElementType { TEXT, CHARACTER, BACKGROUND };

struct Element {
    ElementType type;
    std::string name;
    std::variant<TextElement, CharacterElement, BackgroundElement> data;

    Element() : type(ElementType::TEXT), name("New Element"), data(TextElement{""}) {}
    ~Element() = default;
};

// Scene Element with timing for rendering
struct SceneElement {
    size_t elementIndex; // Index into the elements vector
    float startTime;    // When the element starts rendering (in seconds)
    float endTime;      // When the element stops rendering (in seconds)
};

// Scene structure
struct Scene {
    std::string name;
    std::vector<SceneElement> elements;
};

// File dialog using zenity
std::string OpenFileDialog() {
    char filename[1024] = "";
    FILE* f = popen("zenity --file-selection", "r");
    if (f) {
        fgets(filename, 1024, f);
        pclose(f);
        size_t len = strlen(filename);
        if (len > 0 && filename[len - 1] == '\n') filename[len - 1] = '\0';
    }
    return std::string(filename);
}

// Validate image path
bool IsValidImagePath(const std::string& path) {
    if (path.empty()) return false;
    std::filesystem::path fsPath(path);
    return std::filesystem::exists(fsPath) &&
           (fsPath.extension() == ".png" || fsPath.extension() == ".jpg" || fsPath.extension() == ".jpeg");
}

// Main application class
class ElementCreator {
public:
    ElementCreator() {
        InitWindow(1000, 600, "Novel Scene Creator");
        SetTargetFPS(60);
        currentElementIndex = -1;
        currentSceneIndex = -1;
        currentSceneElementIndex = -1;
        prevSceneElementIndex = -1;
        focusedTextBox = -1;
        isEditing = false;
        elementScrollOffset = 0.0f;
        sceneScrollOffset = 0.0f;
        sceneElementScrollOffset = 0.0f;
        currentMode = Mode::ELEMENT;
    }

    ~ElementCreator() {
        // Unload all textures before closing
        for (auto& element : elements) {
            if (element.type == ElementType::CHARACTER) {
                auto& character = std::get<CharacterElement>(element.data);
                for (auto& texture : character.textures) {
                    if (texture.id > 0) UnloadTexture(texture);
                }
            } else if (element.type == ElementType::BACKGROUND) {
                auto& bg = std::get<BackgroundElement>(element.data);
                if (bg.texture.id > 0) UnloadTexture(bg.texture);
            }
        }
        CloseWindow();
    }

    void run() {
        while (!WindowShouldClose()) {
            update();
            draw();
        }
    }

private:
    enum class Mode { ELEMENT, SCENE };
    Mode currentMode;
    std::vector<Element> elements;
    std::vector<Scene> scenes;
    int currentElementIndex;
    int currentSceneIndex;
    int currentSceneElementIndex;
    int prevSceneElementIndex; // Track previous selection to avoid spamming
    char nameBuffer[256] = "";
    char textBuffer[1024] = "";
    char charNameBuffer[256] = "";
    char imageNameBuffer[256] = "";
    char imagePathBuffer[256] = "";
    char bgPathBuffer[256] = "";
    char sceneNameBuffer[256] = "";
    char startTimeBuffer[32] = "";
    char endTimeBuffer[32] = "";
    int elementTypeIndex = 0;
    bool showAddImage = false;
    bool showEditImage = false;
    int editImageIndex = -1;
    int focusedTextBox; // Tracks which text box is focused (-1 for none)
    bool isEditing;     // Tracks if user is actively editing
    float elementScrollOffset;     // Scroll offset for element list
    float sceneScrollOffset;       // Scroll offset for scene list
    float sceneElementScrollOffset; // Scroll offset for scene elements list

    void update() {
        // Handle mode switching
        if (IsKeyPressed(KEY_TAB)) {
            currentMode = (currentMode == Mode::ELEMENT) ? Mode::SCENE : Mode::ELEMENT;
            currentElementIndex = -1;
            currentSceneIndex = -1;
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
            isEditing = false;
            focusedTextBox = -1;
            clearBuffers();
            TraceLog(LOG_INFO, "Switched to %s mode", currentMode == Mode::ELEMENT ? "Element" : "Scene");
        }

        // Handle scrolling
        float mouseWheelMove = GetMouseWheelMove();
        if (mouseWheelMove != 0) {
            if (currentMode == Mode::ELEMENT) {
                elementScrollOffset -= mouseWheelMove * 20.0f;
                float maxScroll = elements.size() * 40.0f - 500.0f;
                if (maxScroll < 0) maxScroll = 0;
                elementScrollOffset = elementScrollOffset < 0 ? 0 : elementScrollOffset > maxScroll ? maxScroll : elementScrollOffset;
            } else {
                if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){10.0f, 50.0f, 200.0f, 500.0f})) {
                    sceneScrollOffset -= mouseWheelMove * 20.0f;
                    float maxScroll = scenes.size() * 40.0f - 500.0f;
                    if (maxScroll < 0) maxScroll = 0;
                    sceneScrollOffset = sceneScrollOffset < 0 ? 0 : sceneScrollOffset > maxScroll ? maxScroll : sceneScrollOffset;
                } else if (currentSceneIndex >= 0 && CheckCollisionPointRec(GetMousePosition(), (Rectangle){280.0f, 190.0f, 640.0f, 400.0f})) {
                    sceneElementScrollOffset -= mouseWheelMove * 20.0f;
                    float maxScroll = scenes[currentSceneIndex].elements.size() * 40.0f - 400.0f;
                    if (maxScroll < 0) maxScroll = 0;
                    sceneElementScrollOffset = sceneElementScrollOffset < 0 ? 0 : sceneElementScrollOffset > maxScroll ? maxScroll : sceneElementScrollOffset;
                }
            }
        }

        if (currentMode == Mode::ELEMENT) {
            updateElementMode();
        } else {
            updateSceneMode();
        }
    }

    void updateElementMode() {
        if (currentElementIndex == -1 && !isEditing) {
            clearBuffers();
            isEditing = true;
        }

        // Handle text box focus
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            int newFocusedTextBox = -1;

            if (CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 30.0f, 200.0f, 20.0f})) {
                newFocusedTextBox = 0; // Name
            } else if (elementTypeIndex == 0 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 400.0f, 100.0f})) {
                newFocusedTextBox = 1; // Text content
            } else if (elementTypeIndex == 1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 200.0f, 20.0f})) {
                newFocusedTextBox = 2; // Character name
            } else if (elementTypeIndex == 2 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 200.0f, 20.0f})) {
                newFocusedTextBox = 3; // Background path
            } else if ((showAddImage || showEditImage) && CheckCollisionPointRec(mousePos, (Rectangle){410.0f, 220.0f, 180.0f, 20.0f})) {
                newFocusedTextBox = 4; // Image name
            } else if ((showAddImage || showEditImage) && CheckCollisionPointRec(mousePos, (Rectangle){410.0f, 250.0f, 180.0f, 20.0f})) {
                newFocusedTextBox = 5; // Image path
            }

            if (newFocusedTextBox != -1) {
                focusedTextBox = newFocusedTextBox;
                TraceLog(LOG_INFO, "Focused TextBox set to %d", focusedTextBox);
            } else if (!CheckCollisionPointRec(mousePos, (Rectangle){220.0f, 10.0f, 770.0f, 580.0f})) {
                focusedTextBox = -1;
                TraceLog(LOG_INFO, "Focused TextBox cleared");
            }
        }
    }

    void updateSceneMode() {
        if (currentSceneIndex == -1 && !isEditing) {
            strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
            isEditing = true;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            int newFocusedTextBox = -1;

            if (CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 30.0f, 200.0f, 20.0f})) {
                newFocusedTextBox = 6; // Scene name
            } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 100.0f, 20.0f})) {
                newFocusedTextBox = 7; // Start time
            } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 130.0f, 100.0f, 20.0f})) {
                newFocusedTextBox = 8; // End time
            }

            if (newFocusedTextBox != -1) {
                focusedTextBox = newFocusedTextBox;
                TraceLog(LOG_INFO, "Focused TextBox set to %d", focusedTextBox);
            } else if (!CheckCollisionPointRec(mousePos, (Rectangle){220.0f, 10.0f, 770.0f, 580.0f}) &&
                       !CheckCollisionPointRec(mousePos, (Rectangle){280.0f, 190.0f, 640.0f, 400.0f})) {
                focusedTextBox = -1;
                TraceLog(LOG_INFO, "Focused TextBox cleared");
            }
        }
    }

    void clearBuffers() {
        strncpy(nameBuffer, "New Element", sizeof(nameBuffer));
        textBuffer[0] = '\0';
        charNameBuffer[0] = '\0';
        imageNameBuffer[0] = '\0';
        imagePathBuffer[0] = '\0';
        bgPathBuffer[0] = '\0';
        sceneNameBuffer[0] = '\0';
        startTimeBuffer[0] = '\0';
        endTimeBuffer[0] = '\0';
        elementTypeIndex = 0;
        showAddImage = false;
        showEditImage = false;
        editImageIndex = -1;
    }

    void draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Debug info
        DrawText(TextFormat("Mode: %s", currentMode == Mode::ELEMENT ? "Element" : "Scene"), 10, 10, 10, DARKGRAY);
        DrawText(TextFormat("Focused TextBox: %d", focusedTextBox), 10, 20, 10, DARKGRAY);
        DrawText(TextFormat("Is Editing: %d", isEditing), 10, 30, 10, DARKGRAY);
        DrawText(TextFormat("Current Scene Index: %d", currentSceneIndex), 10, 40, 10, DARKGRAY);
        DrawText(TextFormat("Current Scene Element Index: %d", currentSceneElementIndex), 10, 50, 10, DARKGRAY);
        DrawText(TextFormat("Previous Scene Element Index: %d", prevSceneElementIndex), 10, 60, 10, DARKGRAY);

        if (currentMode == Mode::ELEMENT) {
            drawElementMode();
        } else {
            drawSceneMode();
        }

        // Mode toggle hint
        DrawText("Press TAB to switch between Element and Scene modes", 10, 560, 10, DARKGRAY);

        EndDrawing();
    }

    void drawElementMode() {
        // Left panel: Element List
        GuiGroupBox((Rectangle){10.0f, 10.0f, 200.0f, 580.0f}, "Elements");
        BeginScissorMode(10, 50, 200, 500);
        for (size_t i = 0; i < elements.size(); ++i) {
            float yPos = 50.0f + static_cast<float>(i) * 40.0f - elementScrollOffset;
            if (yPos > -40.0f && yPos < 550.0f) {
                if (GuiButton((Rectangle){20.0f, yPos, 180.0f, 30.0f}, elements[i].name.c_str())) {
                    currentElementIndex = i;
                    isEditing = true;
                    loadElementToUI();
                    TraceLog(LOG_INFO, "Selected Element %d", i);
                }
            }
        }
        EndScissorMode();

        // Scrollbar
        float maxScroll = elements.size() * 40.0f - 500.0f;
        if (maxScroll > 0) {
            float scrollBarHeight = 500.0f * (500.0f / (elements.size() * 40.0f));
            float scrollBarY = 50.0f + (elementScrollOffset / maxScroll) * (500.0f - scrollBarHeight);
            DrawRectangle(190, scrollBarY, 10, scrollBarHeight, DARKGRAY);
        }

        // New Element Button
        if (GuiButton((Rectangle){20.0f, 550.0f, 180.0f, 30.0f}, "New Element")) {
            currentElementIndex = -1;
            isEditing = true;
            focusedTextBox = -1;
            clearBuffers();
            TraceLog(LOG_INFO, "Creating new Element");
        }

        // Export Button
        if (GuiButton((Rectangle){20.0f, 510.0f, 180.0f, 30.0f}, "Export to JSON")) {
            exportToJson();
        }

        // Right panel: Element Editor
        GuiGroupBox((Rectangle){220.0f, 10.0f, 770.0f, 580.0f}, "Element Editor");

        if (currentElementIndex == -1 || currentElementIndex < (int)elements.size()) {
            // Name
            GuiLabel((Rectangle){230.0f, 30.0f, 100.0f, 20.0f}, "Element Name:");
            GuiTextBox((Rectangle){340.0f, 30.0f, 200.0f, 20.0f}, nameBuffer, 256, focusedTextBox == 0);

            // Type selection
            GuiLabel((Rectangle){230.0f, 60.0f, 100.0f, 20.0f}, "Element Type:");
            int newTypeIndex = elementTypeIndex;
            GuiComboBox((Rectangle){340.0f, 60.0f, 200.0f, 20.0f}, "Text;Character;Background", &newTypeIndex);
            if (newTypeIndex != elementTypeIndex) {
                elementTypeIndex = newTypeIndex;
                textBuffer[0] = '\0';
                charNameBuffer[0] = '\0';
                bgPathBuffer[0] = '\0';
                imageNameBuffer[0] = '\0';
                imagePathBuffer[0] = '\0';
                showAddImage = false;
                showEditImage = false;
                if (focusedTextBox == 1 && elementTypeIndex != 0) focusedTextBox = -1;
                if (focusedTextBox == 2 && elementTypeIndex != 1) focusedTextBox = -1;
                if (focusedTextBox == 3 && elementTypeIndex != 2) focusedTextBox = -1;
                TraceLog(LOG_INFO, "Element type changed to %d", elementTypeIndex);
            }

            if (elementTypeIndex == 0) { // Text
                GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Content:");
                GuiTextBox((Rectangle){340.0f, 100.0f, 400.0f, 100.0f}, textBuffer, 1024, focusedTextBox == 1);
            } else if (elementTypeIndex == 1) { // Character
                GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Character Name:");
                GuiTextBox((Rectangle){340.0f, 100.0f, 200.0f, 20.0f}, charNameBuffer, 256, focusedTextBox == 2);

                // Image list
                if (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::CHARACTER) {
                    auto& character = std::get<CharacterElement>(elements[currentElementIndex].data);
                    for (size_t i = 0; i < character.images.size(); ++i) {
                        std::string imageInfo = character.images[i].first + ": " + character.images[i].second;
                        GuiLabel((Rectangle){340.0f, 150.0f + static_cast<float>(i) * 60.0f, 300.0f, 20.0f}, imageInfo.c_str());
                        if (i < character.textures.size() && character.textures[i].id > 0) {
                            DrawTexturePro(character.textures[i],
                                           {0, 0, (float)character.textures[i].width, (float)character.textures[i].height},
                                           {440.0f, 150.0f + static_cast<float>(i) * 60.0f, 50.0f, 50.0f},
                                           {0, 0}, 0, WHITE);
                            DrawText(TextFormat("Texture ID: %u", character.textures[i].id),
                                     340, 170 + static_cast<int>(i) * 60, 10, DARKGRAY);
                        }
                        if (GuiButton((Rectangle){650.0f, 150.0f + static_cast<float>(i) * 60.0f, 80.0f, 20.0f}, "Edit")) {
                            showEditImage = true;
                            editImageIndex = i;
                            strncpy(imageNameBuffer, character.images[i].first.c_str(), sizeof(imageNameBuffer));
                            strncpy(imagePathBuffer, character.images[i].second.c_str(), sizeof(imagePathBuffer));
                            TraceLog(LOG_INFO, "Editing image %d for Character", i);
                        }
                    }
                }

                // Add image button
                float imageButtonY = 150.0f + (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::CHARACTER
                    ? static_cast<float>(std::get<CharacterElement>(elements[currentElementIndex].data).images.size()) * 60.0f
                    : 0.0f);
                if (GuiButton((Rectangle){340.0f, imageButtonY, 100.0f, 20.0f}, "Add Image")) {
                    showAddImage = true;
                    imageNameBuffer[0] = '\0';
                    imagePathBuffer[0] = '\0';
                    TraceLog(LOG_INFO, "Adding new image for Character");
                }

                // Add/Edit image popup
                if (showAddImage || showEditImage) {
                    const char* title = showAddImage ? "Add Image" : "Edit Image";
                    GuiGroupBox((Rectangle){300.0f, 200.0f, 300.0f, 200.0f}, title);
                    GuiLabel((Rectangle){310.0f, 220.0f, 100.0f, 20.0f}, "Image Name:");
                    GuiTextBox((Rectangle){410.0f, 220.0f, 180.0f, 20.0f}, imageNameBuffer, 256, focusedTextBox == 4);
                    GuiLabel((Rectangle){310.0f, 250.0f, 100.0f, 20.0f}, "Image Path:");
                    GuiTextBox((Rectangle){410.0f, 250.0f, 180.0f, 20.0f}, imagePathBuffer, 256, focusedTextBox == 5);
                    if (GuiButton((Rectangle){310.0f, 280.0f, 90.0f, 20.0f}, "Select File")) {
                        std::string file = OpenFileDialog();
                        if (!file.empty()) {
                            strncpy(imagePathBuffer, file.c_str(), sizeof(imagePathBuffer));
                            TraceLog(LOG_INFO, "Selected image file: %s", imagePathBuffer);
                        }
                    }
                    if (GuiButton((Rectangle){310.0f, 310.0f, 90.0f, 20.0f}, showAddImage ? "Add" : "Save")) {
                        if (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::CHARACTER) {
                            auto& character = std::get<CharacterElement>(elements[currentElementIndex].data);
                            if (showAddImage && IsValidImagePath(imagePathBuffer)) {
                                character.images.emplace_back(imageNameBuffer, imagePathBuffer);
                                Texture2D texture = LoadTexture(imagePathBuffer);
                                if (texture.id > 0) {
                                    character.textures.push_back(texture);
                                    TraceLog(LOG_INFO, "Loaded texture ID %u for path %s", texture.id, imagePathBuffer);
                                } else {
                                    TraceLog(LOG_WARNING, "Failed to load texture for path %s", imagePathBuffer);
                                    character.textures.push_back({0});
                                }
                            } else if (showEditImage && editImageIndex >= 0 && editImageIndex < (int)character.images.size() && IsValidImagePath(imagePathBuffer)) {
                                character.images[editImageIndex] = {imageNameBuffer, imagePathBuffer};
                                if (editImageIndex < (int)character.textures.size() && character.textures[editImageIndex].id > 0) {
                                    UnloadTexture(character.textures[editImageIndex]);
                                }
                                Texture2D texture = LoadTexture(imagePathBuffer);
                                if (texture.id > 0) {
                                    if (editImageIndex < (int)character.textures.size()) {
                                        character.textures[editImageIndex] = texture;
                                    } else {
                                        character.textures.push_back(texture);
                                    }
                                    TraceLog(LOG_INFO, "Updated texture ID %u for path %s", texture.id, imagePathBuffer);
                                } else {
                                    TraceLog(LOG_WARNING, "Failed to update texture for path %s", imagePathBuffer);
                                    if (editImageIndex >= (int)character.textures.size()) {
                                        character.textures.push_back({0});
                                    } else {
                                        character.textures[editImageIndex] = {0};
                                    }
                                }
                            }
                        }
                        showAddImage = false;
                        showEditImage = false;
                        imageNameBuffer[0] = '\0';
                        imagePathBuffer[0] = '\0';
                    }
                    if (GuiButton((Rectangle){410.0f, 310.0f, 90.0f, 20.0f}, "Cancel")) {
                        showAddImage = false;
                        showEditImage = false;
                        imageNameBuffer[0] = '\0';
                        imagePathBuffer[0] = '\0';
                    }
                }
            } else { // Background
                GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Image Path:");
                GuiTextBox((Rectangle){340.0f, 100.0f, 200.0f, 20.0f}, bgPathBuffer, 256, focusedTextBox == 3);
                if (GuiButton((Rectangle){550.0f, 100.0f, 100.0f, 20.0f}, "Select File")) {
                    std::string file = OpenFileDialog();
                    if (!file.empty()) {
                        strncpy(bgPathBuffer, file.c_str(), sizeof(bgPathBuffer));
                        TraceLog(LOG_INFO, "Selected background file: %s", bgPathBuffer);
                    }
                }
                if (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::BACKGROUND) {
                    auto& bg = std::get<BackgroundElement>(elements[currentElementIndex].data);
                    if (bg.texture.id > 0) {
                        DrawTexturePro(bg.texture,
                                       {0, 0, (float)bg.texture.width, (float)bg.texture.height},
                                       {340.0f, 130.0f, 100.0f, 100.0f},
                                       {0, 0}, 0, WHITE);
                        DrawText(TextFormat("Texture ID: %u", bg.texture.id), 340, 230, 10, DARKGRAY);
                    }
                }
            }

            // Save Button
            if (GuiButton((Rectangle){340.0f, 550.0f, 100.0f, 20.0f}, currentElementIndex == -1 ? "Create" : "Save")) {
                saveElement();
                isEditing = false;
                loadElementToUI();
                TraceLog(LOG_INFO, "Saved Element %d", currentElementIndex);
            }
        }
    }

    void drawSceneMode() {
        // Left panel: Scene List
        GuiGroupBox((Rectangle){10.0f, 10.0f, 200.0f, 580.0f}, "Scenes");
        BeginScissorMode(10, 50, 200, 500);
        for (size_t i = 0; i < scenes.size(); ++i) {
            float yPos = 50.0f + static_cast<float>(i) * 40.0f - sceneScrollOffset;
            if (yPos > -40.0f && yPos < 550.0f) {
                if (GuiButton((Rectangle){20.0f, yPos, 180.0f, 30.0f}, scenes[i].name.c_str())) {
                    currentSceneIndex = i;
                    currentSceneElementIndex = -1;
                    prevSceneElementIndex = -1;
                    isEditing = true;
                    focusedTextBox = -1;
                    loadSceneToUI();
                    TraceLog(LOG_INFO, "Selected Scene %d", currentSceneIndex);
                }
            }
        }
        EndScissorMode();

        // Scrollbar for scenes
        float maxScroll = scenes.size() * 40.0f - 500.0f;
        if (maxScroll > 0) {
            float scrollBarHeight = 500.0f * (500.0f / (scenes.size() * 40.0f));
            float scrollBarY = 50.0f + (sceneScrollOffset / maxScroll) * (500.0f - scrollBarHeight);
            DrawRectangle(190, scrollBarY, 10, scrollBarHeight, DARKGRAY);
        }

        // New Scene Button
        if (GuiButton((Rectangle){20.0f, 550.0f, 180.0f, 30.0f}, "New Scene")) {
            currentSceneIndex = -1;
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
            isEditing = true;
            focusedTextBox = -1;
            strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            TraceLog(LOG_INFO, "Creating new Scene");
        }

        // Export Button
        if (GuiButton((Rectangle){20.0f, 510.0f, 180.0f, 30.0f}, "Export to JSON")) {
            exportToJson();
        }

        // Right panel: Scene Editor
        GuiGroupBox((Rectangle){220.0f, 10.0f, 770.0f, 580.0f}, "Scene Editor");

        if (currentSceneIndex == -1 || currentSceneIndex < (int)scenes.size()) {
            // Scene Name
            GuiLabel((Rectangle){230.0f, 30.0f, 100.0f, 20.0f}, "Scene Name:");
            GuiTextBox((Rectangle){340.0f, 30.0f, 200.0f, 20.0f}, sceneNameBuffer, 256, focusedTextBox == 6);

            // Scene Elements List
            GuiLabel((Rectangle){230.0f, 160.0f, 100.0f, 20.0f}, "Scene Elements:");
            BeginScissorMode(280, 190, 640, 400);
            if (currentSceneIndex >= 0) {
                for (size_t i = 0; i < scenes[currentSceneIndex].elements.size(); ++i) {
                    float yPos = 190.0f + static_cast<float>(i) * 40.0f - sceneElementScrollOffset;
                    if (yPos > -40.0f && yPos < 590.0f) {
                        size_t elemIndex = scenes[currentSceneIndex].elements[i].elementIndex;
                        if (elemIndex < elements.size()) {
                            std::string elementInfo = elements[elemIndex].name +
                                TextFormat(" [%.1f - %.1f]",
                                         scenes[currentSceneIndex].elements[i].startTime,
                                         scenes[currentSceneIndex].elements[i].endTime);
                            Color buttonColor = (static_cast<int>(i) == currentSceneElementIndex) ? SKYBLUE : LIGHTGRAY;
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(buttonColor));
                            if (GuiButton((Rectangle){280.0f, yPos, 300.0f, 30.0f}, elementInfo.c_str())) {
                                TraceLog(LOG_INFO, "Clicked SceneElement %d (ElementIndex=%zu)", i, elemIndex);
                                currentSceneElementIndex = i;
                                isEditing = true;
                                focusedTextBox = -1; // Clear focus to ensure button clicks work
                                loadSceneElementToUI();
                            }
                            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
                        }
                    }
                }
            }
            EndScissorMode();
            // Debug outline for SceneElement list
            DrawRectangleLines(280, 190, 640, 400, RED);

            // Scrollbar for scene elements
            float sceneMaxScroll = currentSceneIndex >= 0 ? scenes[currentSceneIndex].elements.size() * 40.0f - 400.0f : 0;
            if (sceneMaxScroll > 0) {
                float scrollBarHeight = 400.0f * (400.0f / (scenes[currentSceneIndex].elements.size() * 40.0f));
                float scrollBarY = 190.0f + (sceneElementScrollOffset / sceneMaxScroll) * (400.0f - scrollBarHeight);
                DrawRectangle(900, scrollBarY, 10, scrollBarHeight, DARKGRAY);
            }

            // Add Element to Scene
            if (GuiButton((Rectangle){850.0f, 30.0f, 120.0f, 20.0f}, "Add Element")) {
                currentSceneElementIndex = -1;
                prevSceneElementIndex = -1;
                startTimeBuffer[0] = '\0';
                endTimeBuffer[0] = '\0';
                isEditing = true;
                focusedTextBox = -1;
                TraceLog(LOG_INFO, "Adding new SceneElement");
            }

            // Sort Elements Button
            if (currentSceneIndex >= 0 && GuiButton((Rectangle){850.0f, 60.0f, 120.0f, 20.0f}, "Sort Elements")) {
                sortSceneElements();
                if (currentSceneElementIndex >= 0 && currentSceneElementIndex < (int)scenes[currentSceneIndex].elements.size()) {
                    loadSceneElementToUI();
                } else {
                    currentSceneElementIndex = -1;
                    prevSceneElementIndex = -1;
                    startTimeBuffer[0] = '\0';
                    endTimeBuffer[0] = '\0';
                }
                TraceLog(LOG_INFO, "Sorted SceneElements");
            }

            // Scene Element Editor
            if (currentSceneElementIndex >= -1 && (currentSceneIndex >= 0 || currentSceneIndex == -1)) {
                // Element selection
                std::string elementNames = elements.empty() ? "No Elements" : "";
                for (size_t i = 0; i < elements.size(); ++i) {
                    elementNames += elements[i].name;
                    if (i < elements.size() - 1) elementNames += ";";
                }
                static int selectedElement = 0;
                // Update selectedElement only when currentSceneElementIndex changes
                if (currentSceneElementIndex != prevSceneElementIndex) {
                    if (currentSceneIndex >= 0 && currentSceneElementIndex >= 0 &&
                        currentSceneElementIndex < (int)scenes[currentSceneIndex].elements.size()) {
                        selectedElement = scenes[currentSceneIndex].elements[currentSceneElementIndex].elementIndex;
                        TraceLog(LOG_INFO, "Set selectedElement to %d for SceneElement %d", selectedElement, currentSceneElementIndex);
                    } else if (currentSceneElementIndex == -1 && !elements.empty()) {
                        selectedElement = 0;
                        TraceLog(LOG_INFO, "Reset selectedElement to 0 for new SceneElement");
                    }
                    prevSceneElementIndex = currentSceneElementIndex;
                }
                int prevSelectedElement = selectedElement;
                GuiComboBox((Rectangle){340.0f, 70.0f, 200.0f, 20.0f}, elementNames.c_str(), &selectedElement);
                if (prevSelectedElement != selectedElement) {
                    TraceLog(LOG_INFO, "Element dropdown changed to %d (prev=%d)", selectedElement, prevSelectedElement);
                }

                GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Start Time (s):");
                GuiTextBox((Rectangle){340.0f, 100.0f, 100.0f, 20.0f}, startTimeBuffer, 32, focusedTextBox == 7);
                GuiLabel((Rectangle){230.0f, 130.0f, 100.0f, 20.0f}, "End Time (s):");
                GuiTextBox((Rectangle){340.0f, 130.0f, 100.0f, 20.0f}, endTimeBuffer, 32, focusedTextBox == 8);

                // Save Scene Element
                if (!elements.empty() && GuiButton((Rectangle){850.0f, 90.0f, 100.0f, 20.0f}, currentSceneElementIndex == -1 ? "Add" : "Save")) {
                    saveSceneElement(selectedElement);
                    isEditing = false;
                    focusedTextBox = -1;
                    if (currentSceneElementIndex == -1) {
                        currentSceneElementIndex = scenes[currentSceneIndex].elements.size() - 1;
                        prevSceneElementIndex = currentSceneElementIndex;
                    }
                    loadSceneElementToUI();
                    TraceLog(LOG_INFO, "Saved SceneElement, set currentSceneElementIndex to %d", currentSceneElementIndex);
                }
            }
        }
    }

    void loadElementToUI() {
        if (currentElementIndex < 0 || currentElementIndex >= (int)elements.size()) {
            if (!isEditing) {
                clearBuffers();
            }
            return;
        }

        Element& element = elements[currentElementIndex];
        strncpy(nameBuffer, element.name.c_str(), sizeof(nameBuffer));
        elementTypeIndex = static_cast<int>(element.type);

        if (element.type == ElementType::TEXT) {
            strncpy(textBuffer, std::get<TextElement>(element.data).content.c_str(), sizeof(textBuffer));
            charNameBuffer[0] = '\0';
            bgPathBuffer[0] = '\0';
        } else if (element.type == ElementType::CHARACTER) {
            auto& character = std::get<CharacterElement>(element.data);
            strncpy(charNameBuffer, character.name.c_str(), sizeof(charNameBuffer));
            textBuffer[0] = '\0';
            bgPathBuffer[0] = '\0';
            if (character.textures.size() != character.images.size()) {
                for (auto& texture : character.textures) {
                    if (texture.id > 0) UnloadTexture(texture);
                }
                character.textures.clear();
                for (const auto& img : character.images) {
                    Texture2D texture = {0};
                    if (IsValidImagePath(img.second)) {
                        texture = LoadTexture(img.second.c_str());
                        if (texture.id > 0) {
                            TraceLog(LOG_INFO, "Loaded texture ID %u for path %s", texture.id, img.second.c_str());
                        } else {
                            TraceLog(LOG_WARNING, "Failed to load texture for path %s", img.second.c_str());
                        }
                    }
                    character.textures.push_back(texture);
                }
            }
        } else {
            auto& bg = std::get<BackgroundElement>(element.data);
            strncpy(bgPathBuffer, bg.imagePath.c_str(), sizeof(bgPathBuffer));
            textBuffer[0] = '\0';
            charNameBuffer[0] = '\0';
            if (bg.texture.id == 0 && IsValidImagePath(bg.imagePath)) {
                bg.texture = LoadTexture(bg.imagePath.c_str());
                if (bg.texture.id > 0) {
                    TraceLog(LOG_INFO, "Loaded texture ID %u for path %s", bg.texture.id, bg.imagePath.c_str());
                } else {
                    TraceLog(LOG_WARNING, "Failed to load texture for path %s", bg.imagePath.c_str());
                }
            }
        }
        imageNameBuffer[0] = '\0';
        imagePathBuffer[0] = '\0';
        showAddImage = false;
        showEditImage = false;
    }

    void loadSceneToUI() {
        TraceLog(LOG_INFO, "Loading Scene %d", currentSceneIndex);
        if (currentSceneIndex < 0 || currentSceneIndex >= (int)scenes.size()) {
            strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
            return;
        }

        Scene& scene = scenes[currentSceneIndex];
        strncpy(sceneNameBuffer, scene.name.c_str(), sizeof(sceneNameBuffer));
        if (currentSceneElementIndex >= 0 && currentSceneElementIndex < (int)scene.elements.size()) {
            loadSceneElementToUI();
        } else {
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
        }
    }

    void loadSceneElementToUI() {
        if (currentSceneIndex < 0 || currentSceneElementIndex < 0 ||
            currentSceneElementIndex >= (int)scenes[currentSceneIndex].elements.size()) {
            TraceLog(LOG_INFO, "Clearing SceneElement UI (invalid selection: Scene=%d, SceneElement=%d)",
                     currentSceneIndex, currentSceneElementIndex);
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            return;
        }

        SceneElement& sceneElement = scenes[currentSceneIndex].elements[currentSceneElementIndex];
        snprintf(startTimeBuffer, sizeof(startTimeBuffer), "%.1f", sceneElement.startTime);
        snprintf(endTimeBuffer, sizeof(endTimeBuffer), "%.1f", sceneElement.endTime);
        TraceLog(LOG_INFO, "Loaded SceneElement %d: ElementIndex=%zu, Start=%.1f, End=%.1f",
                 currentSceneElementIndex, sceneElement.elementIndex, sceneElement.startTime, sceneElement.endTime);
    }

    void saveElement() {
        Element element;
        element.name = nameBuffer;
        element.type = static_cast<ElementType>(elementTypeIndex);

        if (elementTypeIndex == 0) {
            element.data = TextElement{textBuffer};
        } else if (elementTypeIndex == 1) {
            CharacterElement character;
            character.name = charNameBuffer;
            if (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::CHARACTER) {
                character.images = std::get<CharacterElement>(elements[currentElementIndex].data).images;
                character.textures = std::get<CharacterElement>(elements[currentElementIndex].data).textures;
            }
            element.data = character;
        } else {
            BackgroundElement bg;
            bg.imagePath = bgPathBuffer;
            if (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::BACKGROUND) {
                bg.texture = std::get<BackgroundElement>(elements[currentElementIndex].data).texture;
            }
            if (IsValidImagePath(bgPathBuffer) && bg.texture.id == 0) {
                bg.texture = LoadTexture(bgPathBuffer);
                if (bg.texture.id > 0) {
                    TraceLog(LOG_INFO, "Loaded texture ID %u for path %s", bg.texture.id, bgPathBuffer);
                } else {
                    TraceLog(LOG_WARNING, "Failed to load texture for path %s", bgPathBuffer);
                }
            }
            element.data = bg;
        }

        if (currentElementIndex == -1) {
            elements.push_back(element);
            currentElementIndex = elements.size() - 1;
        } else {
            if (elements[currentElementIndex].type != element.type) {
                if (elements[currentElementIndex].type == ElementType::CHARACTER) {
                    auto& oldChar = std::get<CharacterElement>(elements[currentElementIndex].data);
                    for (auto& texture : oldChar.textures) {
                        if (texture.id > 0) UnloadTexture(texture);
                    }
                } else if (elements[currentElementIndex].type == ElementType::BACKGROUND) {
                    auto& oldBg = std::get<BackgroundElement>(elements[currentElementIndex].data);
                    if (oldBg.texture.id > 0) UnloadTexture(oldBg.texture);
                }
            } else if (element.type == ElementType::BACKGROUND) {
                auto& oldBg = std::get<BackgroundElement>(elements[currentElementIndex].data);
                auto& newBg = std::get<BackgroundElement>(element.data);
                if (oldBg.imagePath != newBg.imagePath && oldBg.texture.id > 0) {
                    UnloadTexture(oldBg.texture);
                    newBg.texture = {0};
                }
            }
            elements[currentElementIndex] = element;
        }
    }

    void saveSceneElement(size_t selectedElementIndex) {
        if (selectedElementIndex >= elements.size()) {
            TraceLog(LOG_WARNING, "Invalid selectedElementIndex %zu", selectedElementIndex);
            return;
        }

        SceneElement sceneElement;
        sceneElement.elementIndex = selectedElementIndex;

        try {
            sceneElement.startTime = std::stof(startTimeBuffer);
            sceneElement.endTime = std::stof(endTimeBuffer);
        } catch (...) {
            sceneElement.startTime = 0.0f;
            sceneElement.endTime = 1.0f;
        }

        if (sceneElement.endTime <= sceneElement.startTime) {
            sceneElement.endTime = sceneElement.startTime + 1.0f;
        }

        if (currentSceneIndex == -1) {
            Scene newScene;
            newScene.name = sceneNameBuffer;
            newScene.elements.push_back(sceneElement);
            scenes.push_back(newScene);
            currentSceneIndex = scenes.size() - 1;
            currentSceneElementIndex = 0;
            prevSceneElementIndex = 0;
        } else {
            if (currentSceneElementIndex == -1) {
                scenes[currentSceneIndex].elements.push_back(sceneElement);
                currentSceneElementIndex = scenes[currentSceneIndex].elements.size() - 1;
                prevSceneElementIndex = currentSceneElementIndex;
            } else {
                scenes[currentSceneIndex].elements[currentSceneElementIndex] = sceneElement;
            }
            scenes[currentSceneIndex].name = sceneNameBuffer;
        }
        TraceLog(LOG_INFO, "Saved SceneElement: Index=%d, ElementIndex=%zu, Start=%.1f, End=%.1f",
                 currentSceneElementIndex, sceneElement.elementIndex, sceneElement.startTime, sceneElement.endTime);
        loadSceneElementToUI();
    }

    void sortSceneElements() {
        if (currentSceneIndex < 0 || currentSceneIndex >= (int)scenes.size()) return;

        std::sort(scenes[currentSceneIndex].elements.begin(),
                  scenes[currentSceneIndex].elements.end(),
                  [this](const SceneElement& a, const SceneElement& b) {
                      if (a.startTime != b.startTime) return a.startTime < b.startTime;
                      return a.endTime < b.endTime;
                  });
        TraceLog(LOG_INFO, "Sorted SceneElements for Scene %d", currentSceneIndex);
    }

    void exportToJson() {
        json j_export;
        json j_elements = json::array();
        json j_scenes = json::array();

        // Export elements
        for (const auto& element : elements) {
            json j_element;
            j_element["name"] = element.name;

            if (element.type == ElementType::TEXT) {
                j_element["type"] = "text";
                j_element["content"] = std::get<TextElement>(element.data).content;
            } else if (element.type == ElementType::CHARACTER) {
                j_element["type"] = "character";
                j_element["character_name"] = std::get<CharacterElement>(element.data).name;
                json j_images = json::array();
                for (const auto& img : std::get<CharacterElement>(element.data).images) {
                    j_images.push_back({{"name", img.first}, {"path", img.second}});
                }
                j_element["images"] = j_images;
            } else {
                j_element["type"] = "background";
                j_element["image_path"] = std::get<BackgroundElement>(element.data).imagePath;
            }

            j_elements.push_back(j_element);
        }

        // Export scenes
        for (const auto& scene : scenes) {
            json j_scene;
            j_scene["name"] = scene.name;
            json j_scene_elements = json::array();
            for (const auto& sceneElement : scene.elements) {
                if (sceneElement.elementIndex < elements.size()) {
                    json j_scene_element;
                    j_scene_element["element_name"] = elements[sceneElement.elementIndex].name;
                    j_scene_element["start_time"] = sceneElement.startTime;
                    j_scene_element["end_time"] = sceneElement.endTime;
                    j_scene_elements.push_back(j_scene_element);
                }
            }
            j_scene["elements"] = j_scene_elements;
            j_scenes.push_back(j_scene);
        }

        j_export["elements"] = j_elements;
        j_export["scenes"] = j_scenes;

        std::ofstream file("elements_and_scenes.json");
        file << j_export.dump(4);
        file.close();
        TraceLog(LOG_INFO, "Exported to elements_and_scenes.json");
    }
};

int main() {
    ElementCreator app;
    app.run();
    return 0;
}
