#define RAYGUI_IMPLEMENTATION
// #include "raygui.h"
#include "ElementEditor.hpp"
#include "FileUtils.hpp"
#include <algorithm>
#include <fstream>

ElementEditor::ElementEditor() {
    currentElementIndex = -1;
    focusedTextBox = -1;
    isEditing = false;
    elementScrollOffset = 0.0f;
    elementTypeIndex = 0;
    showAddImage = false;
    showEditImage = false;
    editImageIndex = -1;
    nameBuffer[0] = '\0';
    textBuffer[0] = '\0';
    charNameBuffer[0] = '\0';
    imageNameBuffer[0] = '\0';
    imagePathBuffer[0] = '\0';
    bgPathBuffer[0] = '\0';
}

ElementEditor::~ElementEditor() {
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
}

std::vector<Element>& ElementEditor::getElements() {
    return elements;
}

std::vector<Scene>& ElementEditor::getScenes() {
    return scenes;
}

void ElementEditor::update() {
    updateElementMode();
}

void ElementEditor::updateElementMode() {
    if (currentElementIndex == -1 && !isEditing) {
        clearBuffers();
        isEditing = true;
    }

    float mouseWheelMove = GetMouseWheelMove();
    if (mouseWheelMove != 0) {
        elementScrollOffset -= mouseWheelMove * 20.0f;
        float maxScroll = elements.size() * 40.0f - 500.0f;
        if (maxScroll < 0) maxScroll = 0;
        elementScrollOffset = elementScrollOffset < 0 ? 0 : elementScrollOffset > maxScroll ? maxScroll : elementScrollOffset;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        int newFocusedTextBox = -1;

        if (CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 30.0f, 200.0f, 20.0f})) {
            newFocusedTextBox = 0;
        } else if (elementTypeIndex == 0 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 400.0f, 100.0f})) {
            newFocusedTextBox = 1;
        } else if (elementTypeIndex == 1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 200.0f, 20.0f})) {
            newFocusedTextBox = 2;
        } else if (elementTypeIndex == 2 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 200.0f, 20.0f})) {
            newFocusedTextBox = 3;
        } else if ((showAddImage || showEditImage) && CheckCollisionPointRec(mousePos, (Rectangle){410.0f, 220.0f, 180.0f, 20.0f})) {
            newFocusedTextBox = 4;
        } else if ((showAddImage || showEditImage) && CheckCollisionPointRec(mousePos, (Rectangle){410.0f, 250.0f, 180.0f, 20.0f})) {
            newFocusedTextBox = 5;
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

void ElementEditor::clearBuffers() {
    strncpy(nameBuffer, "New Element", sizeof(nameBuffer));
    textBuffer[0] = '\0';
    charNameBuffer[0] = '\0';
    imageNameBuffer[0] = '\0';
    imagePathBuffer[0] = '\0';
    bgPathBuffer[0] = '\0';
    elementTypeIndex = 0;
    showAddImage = false;
    showEditImage = false;
    editImageIndex = -1;
}

void ElementEditor::draw() {
    DrawText("Mode: Element", 10, 10, 10, DARKGRAY);
    DrawText(TextFormat("Focused TextBox: %d", focusedTextBox), 10, 20, 10, DARKGRAY);
    DrawText(TextFormat("Is Editing: %d", isEditing), 10, 30, 10, DARKGRAY);
    drawElementMode();
}

void ElementEditor::drawElementMode() {
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

    float maxScroll = elements.size() * 40.0f - 500.0f;
    if (maxScroll > 0) {
        float scrollBarHeight = 500.0f * (500.0f / (elements.size() * 40.0f));
        float scrollBarY = 50.0f + (elementScrollOffset / maxScroll) * (500.0f - scrollBarHeight);
        DrawRectangle(190, scrollBarY, 10, scrollBarHeight, DARKGRAY);
    }

    if (GuiButton((Rectangle){20.0f, 550.0f, 180.0f, 30.0f}, "New Element")) {
        currentElementIndex = -1;
        isEditing = true;
        focusedTextBox = -1;
        clearBuffers();
        TraceLog(LOG_INFO, "Creating new Element");
    }

    if (GuiButton((Rectangle){20.0f, 510.0f, 180.0f, 30.0f}, "Export to JSON")) {
        exportToJson();
    }

    GuiGroupBox((Rectangle){220.0f, 10.0f, 770.0f, 580.0f}, "Element Editor");

    if (currentElementIndex == -1 || currentElementIndex < (int)elements.size()) {
        GuiLabel((Rectangle){230.0f, 30.0f, 100.0f, 20.0f}, "Element Name:");
        GuiTextBox((Rectangle){340.0f, 30.0f, 200.0f, 20.0f}, nameBuffer, 256, focusedTextBox == 0);

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

        if (elementTypeIndex == 0) {
            GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Content:");
            GuiTextBox((Rectangle){340.0f, 100.0f, 400.0f, 100.0f}, textBuffer, 1024, focusedTextBox == 1);
        } else if (elementTypeIndex == 1) {
            GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Character Name:");
            GuiTextBox((Rectangle){340.0f, 100.0f, 200.0f, 20.0f}, charNameBuffer, 256, focusedTextBox == 2);

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

            float imageButtonY = 150.0f + (currentElementIndex >= 0 && elements[currentElementIndex].type == ElementType::CHARACTER
                ? static_cast<float>(std::get<CharacterElement>(elements[currentElementIndex].data).images.size()) * 60.0f
                : 0.0f);
            if (GuiButton((Rectangle){340.0f, imageButtonY, 100.0f, 20.0f}, "Add Image")) {
                showAddImage = true;
                imageNameBuffer[0] = '\0';
                imagePathBuffer[0] = '\0';
                TraceLog(LOG_INFO, "Adding new image for Character");
            }

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
        } else {
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

        if (GuiButton((Rectangle){340.0f, 550.0f, 100.0f, 20.0f}, currentElementIndex == -1 ? "Create" : "Save")) {
            saveElement();
            isEditing = false;
            loadElementToUI();
            TraceLog(LOG_INFO, "Saved Element %d", currentElementIndex);
        }
    }
}

void ElementEditor::loadElementToUI() {
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

void ElementEditor::saveElement() {
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

void ElementEditor::exportToJson() {
    json j_export;
    json j_elements = json::array();
    json j_scenes = json::array();

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
