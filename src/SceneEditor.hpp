#ifndef SCENE_EDITOR_HPP
#define SCENE_EDITOR_HPP

#include "raylib.h"
#include "raygui.h"

#include "Types.hpp"
#include "BasicUI.hpp"

#include <vector>
#include <string>


class SceneEditor : BasicUI
{
public:
    SceneEditor(std::vector<Element>& elements, std::vector<Scene>& scenes);
    void update();
    void draw();
    std::vector<Scene>& getScenes();

private:
    std::vector<Element>& elements; // Reference to shared elements
    std::vector<Scene>& scenes;     // Reference to shared scenes
    int currentSceneIndex;
    int currentSceneElementIndex;
    int prevSceneElementIndex;
    char sceneNameBuffer[256];
    char startTimeBuffer[32];
    char endTimeBuffer[32];
    int focusedTextBox;
    bool isEditing;
    float sceneScrollOffset;
    float sceneElementScrollOffset;

    void updateSceneMode();

    void clearBuffers();
    void drawSceneMode();
    void loadSceneToUI();
    void loadSceneElementToUI();
    void saveSceneElement(size_t selectedElementIndex);
    void sortSceneElements();
    void exportToJson();
};

#endif // SCENE_EDITOR_HPP
