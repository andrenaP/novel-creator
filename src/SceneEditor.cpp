#include "SceneEditor.hpp"
#include "FileUtils.hpp"
#include <algorithm>
#include <fstream>

SceneEditor::SceneEditor(std::vector<Element>& elements, std::vector<Scene>& scenes)
    : elements(elements), scenes(scenes) {
    currentSceneIndex = -1;
    currentSceneElementIndex = -1;
    prevSceneElementIndex = -1;
    focusedTextBox = -1;
    isEditing = false;
    sceneScrollOffset = 0.0f;
    sceneElementScrollOffset = 0.0f;
    sceneNameBuffer[0] = '\0';
    startTimeBuffer[0] = '\0';
    endTimeBuffer[0] = '\0';
    renderLevelBuffer[0] = '\0';
    positionIndexBuffer[0] = '\0'; // Added for positionIndex
    poseBuffer[0] = '\0';
}

void SceneEditor::update() {
    updateSceneMode();
}

std::vector<Scene>& SceneEditor::getScenes() {
    return scenes;
}

void SceneEditor::updateSceneMode() {
    if (currentSceneIndex == -1 && !isEditing) {
        strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
        startTimeBuffer[0] = '\0';
        endTimeBuffer[0] = '\0';
        renderLevelBuffer[0] = '\0';
        positionIndexBuffer[0] = '\0'; // Added for positionIndex
        poseBuffer[0] = '\0';
        currentSceneElementIndex = -1;
        prevSceneElementIndex = -1;
        isEditing = true;
    }

    float mouseWheelMove = GetMouseWheelMove();
    if (mouseWheelMove != 0) {
        if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){10.0f, 50.0f, 200.0f, 500.0f})) {
            sceneScrollOffset -= mouseWheelMove * 20.0f;
            float maxScroll = scenes.size() * 40.0f - 500.0f;
            if (maxScroll < 0) maxScroll = 0;
            sceneScrollOffset = sceneScrollOffset < 0 ? 0 : sceneScrollOffset > maxScroll ? maxScroll : sceneScrollOffset;
        } else if (currentSceneIndex >= 0 && CheckCollisionPointRec(GetMousePosition(), (Rectangle){280.0f, 220.0f, 640.0f, 370.0f})) {
            sceneElementScrollOffset -= mouseWheelMove * 20.0f;
            float maxScroll = scenes[currentSceneIndex].elements.size() * 40.0f - 370.0f;
            if (maxScroll < 0) maxScroll = 0;
            sceneElementScrollOffset = sceneElementScrollOffset < 0 ? 0 : sceneElementScrollOffset > maxScroll ? maxScroll : sceneElementScrollOffset;
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        int newFocusedTextBox = -1;

        if (CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 30.0f, 200.0f, 20.0f})) {
            newFocusedTextBox = 6;
        } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 100.0f, 100.0f, 20.0f})) {
            newFocusedTextBox = 7;
        } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 130.0f, 100.0f, 20.0f})) {
            newFocusedTextBox = 8;
        } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 160.0f, 100.0f, 20.0f})) {
            newFocusedTextBox = 9;
        } else if (currentSceneElementIndex >= -1 && CheckCollisionPointRec(mousePos, (Rectangle){340.0f, 190.0f, 100.0f, 20.0f})) {
            newFocusedTextBox = 10; // Added for positionIndex
        }

        if (newFocusedTextBox != -1) {
            focusedTextBox = newFocusedTextBox;
            TraceLog(LOG_INFO, "Focused TextBox set to %d", focusedTextBox);
        } else if (!CheckCollisionPointRec(mousePos, (Rectangle){220.0f, 10.0f, 770.0f, 580.0f}) &&
                   !CheckCollisionPointRec(mousePos, (Rectangle){280.0f, 220.0f, 640.0f, 370.0f})) {
            focusedTextBox = -1;
            TraceLog(LOG_INFO, "Focused TextBox cleared");
        }
    }
}

void SceneEditor::clearBuffers() {
    sceneNameBuffer[0] = '\0';
    startTimeBuffer[0] = '\0';
    endTimeBuffer[0] = '\0';
    renderLevelBuffer[0] = '\0';
    positionIndexBuffer[0] = '\0'; // Added for positionIndex
    poseBuffer[0] = '\0';
}

void SceneEditor::draw() {
    // DrawText("Mode: Scene", 10, 10, 10, DARKGRAY);
    // DrawText(TextFormat("Focused TextBox: %d", focusedTextBox), 10, 20, 10, DARKGRAY);
    // DrawText(TextFormat("Is Editing: %d", isEditing), 10, 30, 10, DARKGRAY);
    // DrawText(TextFormat("Current Scene Index: %d", currentSceneIndex), 10, 40, 10, DARKGRAY);
    // DrawText(TextFormat("Current Scene Element Index: %d", currentSceneElementIndex), 10, 50, 10, DARKGRAY);
    // DrawText(TextFormat("Previous Scene Element Index: %d", prevSceneElementIndex), 10, 60, 10, DARKGRAY);
    drawSceneMode();
}

void SceneEditor::drawSceneMode() {
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

    float maxScroll = scenes.size() * 40.0f - 500.0f;
    if (maxScroll > 0) {
        float scrollBarHeight = 500.0f * (500.0f / (scenes.size() * 40.0f));
        float scrollBarY = 50.0f + (sceneScrollOffset / maxScroll) * (500.0f - scrollBarHeight);
        DrawRectangle(190, scrollBarY, 10, scrollBarHeight, DARKGRAY);
    }

    if (GuiButton((Rectangle){20.0f, 550.0f, 180.0f, 30.0f}, "New Scene")) {
        currentSceneIndex = -1;
        currentSceneElementIndex = -1;
        prevSceneElementIndex = -1;
        isEditing = true;
        focusedTextBox = -1;
        strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
        startTimeBuffer[0] = '\0';
        endTimeBuffer[0] = '\0';
        renderLevelBuffer[0] = '\0';
        positionIndexBuffer[0] = '\0'; // Added for positionIndex
        poseBuffer[0] = '\0';
        TraceLog(LOG_INFO, "Creating new Scene");
    }

    if (GuiButton((Rectangle){20.0f, 510.0f, 180.0f, 30.0f}, "Export to JSON")) {
        exportToJson();
    }

    GuiGroupBox((Rectangle){220.0f, 10.0f, 770.0f, 580.0f}, "Scene Editor");

    if (currentSceneIndex == -1 || currentSceneIndex < (int)scenes.size()) {
        GuiLabel((Rectangle){230.0f, 30.0f, 100.0f, 20.0f}, "Scene Name:");
        GuiTextBox((Rectangle){340.0f, 30.0f, 200.0f, 20.0f}, sceneNameBuffer, 256, focusedTextBox == 6);

        GuiLabel((Rectangle){230.0f, 220.0f, 100.0f, 20.0f}, "Scene Elements:"); // Adjusted Y position
        BeginScissorMode(280, 250, 640, 340); // Adjusted for new text box
        if (currentSceneIndex >= 0) {
            for (size_t i = 0; i < scenes[currentSceneIndex].elements.size(); ++i) {
                float yPos = 250.0f + static_cast<float>(i) * 40.0f - sceneElementScrollOffset;
                if (yPos > -40.0f && yPos < 590.0f) {
                    size_t elemIndex = scenes[currentSceneIndex].elements[i].elementIndex;
                    if (elemIndex < elements.size()) {
                        std::string elementInfo = elements[elemIndex].name +
                            TextFormat(" [%.1f - %.1f, Lvl %d",
                                     scenes[currentSceneIndex].elements[i].startTime,
                                     scenes[currentSceneIndex].elements[i].endTime,
                                     scenes[currentSceneIndex].elements[i].renderlevel);
                        if (elements[elemIndex].type == ElementType::CHARACTER) {
                            elementInfo += TextFormat(", Pos %d", scenes[currentSceneIndex].elements[i].positionIndex);
                            if (!scenes[currentSceneIndex].elements[i].selectedPose.empty()) {
                                elementInfo += ", " + scenes[currentSceneIndex].elements[i].selectedPose;
                            }
                        }
                        elementInfo += "]";
                        Color buttonColor = (static_cast<int>(i) == currentSceneElementIndex) ? SKYBLUE : LIGHTGRAY;
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(buttonColor));
                        if (GuiButton((Rectangle){280.0f, yPos, 300.0f, 30.0f}, elementInfo.c_str())) {
                            TraceLog(LOG_INFO, "Clicked SceneElement %d (ElementIndex=%zu)", i, elemIndex);
                            currentSceneElementIndex = i;
                            isEditing = true;
                            focusedTextBox = -1;
                            loadSceneElementToUI();
                        }
                        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
                    }
                }
            }
        }
        EndScissorMode();
        DrawRectangleLines(280, 250, 640, 340, RED); // Adjusted for new text box

        float sceneMaxScroll = currentSceneIndex >= 0 ? scenes[currentSceneIndex].elements.size() * 40.0f - 340.0f : 0;
        if (sceneMaxScroll > 0) {
            float scrollBarHeight = 340.0f * (340.0f / (scenes[currentSceneIndex].elements.size() * 40.0f));
            float scrollBarY = 250.0f + (sceneElementScrollOffset / sceneMaxScroll) * (340.0f - scrollBarHeight);
            DrawRectangle(900, scrollBarY, 10, scrollBarHeight, DARKGRAY);
        }

        if (GuiButton((Rectangle){850.0f, 30.0f, 120.0f, 20.0f}, "Add Element")) {
            currentSceneElementIndex = -1;
            prevSceneElementIndex = -1;
            startTimeBuffer[0] = '\0';
            endTimeBuffer[0] = '\0';
            renderLevelBuffer[0] = '\0';
            positionIndexBuffer[0] = '\0'; // Added for positionIndex
            poseBuffer[0] = '\0';
            isEditing = true;
            focusedTextBox = -1;
            TraceLog(LOG_INFO, "Adding new SceneElement");
        }

        if (currentSceneIndex >= 0 && GuiButton((Rectangle){850.0f, 60.0f, 120.0f, 20.0f}, "Sort Elements")) {
            sortSceneElements();
            if (currentSceneElementIndex >= 0 && currentSceneElementIndex < (int)scenes[currentSceneIndex].elements.size()) {
                loadSceneElementToUI();
            } else {
                currentSceneElementIndex = -1;
                prevSceneElementIndex = -1;
                startTimeBuffer[0] = '\0';
                endTimeBuffer[0] = '\0';
                renderLevelBuffer[0] = '\0';
                positionIndexBuffer[0] = '\0'; // Added for positionIndex
                poseBuffer[0] = '\0';
            }
            TraceLog(LOG_INFO, "Sorted SceneElements");
        }

        if (currentSceneElementIndex >= -1 && (currentSceneIndex >= 0 || currentSceneIndex == -1)) {
            std::string elementNames = elements.empty() ? "No Elements" : "";
            for (size_t i = 0; i < elements.size(); ++i) {
                elementNames += elements[i].name;
                if (i < elements.size() - 1) elementNames += ";";
            }
            static int selectedElement = 0;
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
                // Reset pose and positionIndex when element changes
                poseBuffer[0] = '\0';
                positionIndexBuffer[0] = '\0'; // Added for positionIndex
            }

            // Pose selection for CharacterElement
            std::string poseNames = "None";
            static int selectedPoseIndex = 0;
            if (selectedElement < elements.size() && elements[selectedElement].type == ElementType::CHARACTER) {
                poseNames = "";
                const auto& character = std::get<CharacterElement>(elements[selectedElement].data);
                for (size_t i = 0; i < character.images.size(); ++i) {
                    poseNames += character.images[i].first;
                    if (i < character.images.size() - 1) poseNames += ";";
                }
                if (currentSceneElementIndex != prevSceneElementIndex && currentSceneIndex >= 0 && currentSceneElementIndex >= 0) {
                    const auto& sceneElement = scenes[currentSceneIndex].elements[currentSceneElementIndex];
                    auto it = std::find_if(character.images.begin(), character.images.end(),
                        [&](const auto& img) { return img.first == sceneElement.selectedPose; });
                    selectedPoseIndex = (it != character.images.end()) ? std::distance(character.images.begin(), it) : 0;
                    strncpy(poseBuffer, sceneElement.selectedPose.c_str(), sizeof(poseBuffer));
                }
            } else {
                selectedPoseIndex = 0;
                poseBuffer[0] = '\0';
            }
            GuiLabel((Rectangle){230.0f, 220.0f, 100.0f, 20.0f}, "Pose:"); // Adjusted Y position
            GuiComboBox((Rectangle){340.0f, 220.0f, 200.0f, 20.0f}, poseNames.c_str(), &selectedPoseIndex);

            GuiLabel((Rectangle){230.0f, 100.0f, 100.0f, 20.0f}, "Start Time (s):");
            GuiTextBox((Rectangle){340.0f, 100.0f, 100.0f, 20.0f}, startTimeBuffer, 32, focusedTextBox == 7);
            GuiLabel((Rectangle){230.0f, 130.0f, 100.0f, 20.0f}, "End Time (s):");
            GuiTextBox((Rectangle){340.0f, 130.0f, 100.0f, 20.0f}, endTimeBuffer, 32, focusedTextBox == 8);
            GuiLabel((Rectangle){230.0f, 160.0f, 100.0f, 20.0f}, "Render Level:");
            GuiTextBox((Rectangle){340.0f, 160.0f, 100.0f, 20.0f}, renderLevelBuffer, 32, focusedTextBox == 9);
            if (selectedElement < elements.size() && elements[selectedElement].type == ElementType::CHARACTER) {
                GuiLabel((Rectangle){230.0f, 190.0f, 100.0f, 20.0f}, "Position Index:");
                GuiTextBox((Rectangle){340.0f, 190.0f, 100.0f, 20.0f}, positionIndexBuffer, 32, focusedTextBox == 10);
            }

            if (!elements.empty() && GuiButton((Rectangle){850.0f, 90.0f, 100.0f, 20.0f}, currentSceneElementIndex == -1 ? "Add" : "Save")) {
                saveSceneElement(selectedElement, selectedPoseIndex);
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

void SceneEditor::loadSceneToUI() {
    TraceLog(LOG_INFO, "Loading Scene %d", currentSceneIndex);
    if (currentSceneIndex < 0 || currentSceneIndex >= (int)scenes.size()) {
        strncpy(sceneNameBuffer, "New Scene", sizeof(sceneNameBuffer));
        startTimeBuffer[0] = '\0';
        endTimeBuffer[0] = '\0';
        renderLevelBuffer[0] = '\0';
        positionIndexBuffer[0] = '\0'; // Added for positionIndex
        poseBuffer[0] = '\0';
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
        renderLevelBuffer[0] = '\0';
        positionIndexBuffer[0] = '\0'; // Added for positionIndex
        poseBuffer[0] = '\0';
        currentSceneElementIndex = -1;
        prevSceneElementIndex = -1;
    }
}

void SceneEditor::loadSceneElementToUI() {
    if (currentSceneIndex < 0 || currentSceneElementIndex < 0 ||
        currentSceneElementIndex >= (int)scenes[currentSceneIndex].elements.size()) {
        TraceLog(LOG_INFO, "Clearing SceneElement UI (invalid selection: Scene=%d, SceneElement=%d)",
                 currentSceneIndex, currentSceneElementIndex);
        startTimeBuffer[0] = '\0';
        endTimeBuffer[0] = '\0';
        renderLevelBuffer[0] = '\0';
        positionIndexBuffer[0] = '\0'; // Added for positionIndex
        poseBuffer[0] = '\0';
        return;
    }

    SceneElement& sceneElement = scenes[currentSceneIndex].elements[currentSceneElementIndex];
    snprintf(startTimeBuffer, sizeof(startTimeBuffer), "%.1f", sceneElement.startTime);
    snprintf(endTimeBuffer, sizeof(endTimeBuffer), "%.1f", sceneElement.endTime);
    snprintf(renderLevelBuffer, sizeof(renderLevelBuffer), "%d", sceneElement.renderlevel);
    snprintf(positionIndexBuffer, sizeof(positionIndexBuffer), "%d", sceneElement.positionIndex); // Added for positionIndex
    strncpy(poseBuffer, sceneElement.selectedPose.c_str(), sizeof(poseBuffer));
    TraceLog(LOG_INFO, "Loaded SceneElement %d: ElementIndex=%zu, Start=%.1f, End=%.1f, RenderLevel=%d, PositionIndex=%d, Pose=%s",
             currentSceneElementIndex, sceneElement.elementIndex, sceneElement.startTime, sceneElement.endTime,
             sceneElement.renderlevel, sceneElement.positionIndex, sceneElement.selectedPose.c_str());
}

void SceneEditor::saveSceneElement(size_t selectedElementIndex, int selectedPoseIndex) {
    if (selectedElementIndex >= elements.size()) {
        TraceLog(LOG_WARNING, "Invalid selectedElementIndex %zu", selectedElementIndex);
        return;
    }

    SceneElement sceneElement;
    sceneElement.elementIndex = selectedElementIndex;

    try {
        sceneElement.startTime = std::stof(startTimeBuffer);
        sceneElement.endTime = std::stof(endTimeBuffer);
        sceneElement.renderlevel = std::stoi(renderLevelBuffer);
        if (elements[selectedElementIndex].type == ElementType::CHARACTER) {
            sceneElement.positionIndex = std::stoi(positionIndexBuffer); // Added for positionIndex
        } else {
            sceneElement.positionIndex = 0; // Default for non-character elements
        }
    } catch (...) {
        sceneElement.startTime = 0.0f;
        sceneElement.endTime = 1.0f;
        sceneElement.renderlevel = 0;
        sceneElement.positionIndex = 0; // Added for positionIndex
    }

    if (sceneElement.endTime <= sceneElement.startTime) {
        sceneElement.endTime = sceneElement.startTime + 1.0f;
    }

    if (elements[selectedElementIndex].type == ElementType::CHARACTER) {
        const auto& character = std::get<CharacterElement>(elements[selectedElementIndex].data);
        if (selectedPoseIndex >= 0 && selectedPoseIndex < (int)character.images.size()) {
            sceneElement.selectedPose = character.images[selectedPoseIndex].first;
        } else {
            sceneElement.selectedPose = character.images.empty() ? "" : character.images[0].first;
        }
    } else {
        sceneElement.selectedPose = "";
        sceneElement.positionIndex = 0; // Ensure non-character elements have positionIndex 0
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
    TraceLog(LOG_INFO, "Saved SceneElement: Index=%d, ElementIndex=%zu, Start=%.1f, End=%.1f, RenderLevel=%d, PositionIndex=%d, Pose=%s",
             currentSceneElementIndex, sceneElement.elementIndex, sceneElement.startTime, sceneElement.endTime,
             sceneElement.renderlevel, sceneElement.positionIndex, sceneElement.selectedPose.c_str());
    loadSceneElementToUI();
}

void SceneEditor::sortSceneElements() {
    if (currentSceneIndex < 0 || currentSceneIndex >= (int)scenes.size()) return;

    std::sort(scenes[currentSceneIndex].elements.begin(),
              scenes[currentSceneIndex].elements.end(),
              [this](const SceneElement& a, const SceneElement& b) {
                  size_t aIndex = a.elementIndex < elements.size() ? a.elementIndex : 0;
                  size_t bIndex = b.elementIndex < elements.size() ? b.elementIndex : 0;
                  bool aIsCharacter = aIndex < elements.size() && elements[aIndex].type == ElementType::CHARACTER;
                  bool bIsCharacter = bIndex < elements.size() && elements[bIndex].type == ElementType::CHARACTER;

                  if (a.startTime != b.startTime) return a.startTime < b.startTime;
                  if (aIsCharacter && bIsCharacter) {
                      if (a.positionIndex != b.positionIndex) return a.positionIndex < b.positionIndex;
                  } else {
                      if (a.renderlevel != b.renderlevel) return a.renderlevel < b.renderlevel;
                  }
                  return a.endTime < b.endTime;
              });
    TraceLog(LOG_INFO, "Sorted SceneElements for Scene %d", currentSceneIndex);
}

void SceneEditor::exportToJson() {
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
                j_scene_element["render_level"] = sceneElement.renderlevel;
                j_scene_element["position_index"] = sceneElement.positionIndex; // Added for positionIndex
                j_scene_element["selected_pose"] = sceneElement.selectedPose;
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
