#ifndef ELEMENT_CREATOR_HPP
#define ELEMENT_CREATOR_HPP

#include "raylib.h"
#include "raygui.h"
#include "Types.hpp"
#include <vector>
#include <string>

class ElementCreator {
public:
    ElementCreator();
    ~ElementCreator();
    void run();

private:
    enum class Mode { ELEMENT, SCENE };
    Mode currentMode;
    std::vector<Element> elements;
    std::vector<Scene> scenes;
    int currentElementIndex;
    int currentSceneIndex;
    int currentSceneElementIndex;
    int prevSceneElementIndex;
    char nameBuffer[256];
    char textBuffer[1024];
    char charNameBuffer[256];
    char imageNameBuffer[256];
    char imagePathBuffer[256];
    char bgPathBuffer[256];
    char sceneNameBuffer[256];
    char startTimeBuffer[32];
    char endTimeBuffer[32];
    int elementTypeIndex;
    bool showAddImage;
    bool showEditImage;
    int editImageIndex;
    int focusedTextBox;
    bool isEditing;
    float elementScrollOffset;
    float sceneScrollOffset;
    float sceneElementScrollOffset;

    void update();
    void updateElementMode();
    void updateSceneMode();
    void clearBuffers();
    void draw();
    void drawElementMode();
    void drawSceneMode();
    void loadElementToUI();
    void loadSceneToUI();
    void loadSceneElementToUI();
    void saveElement();
    void saveSceneElement(size_t selectedElementIndex);
    void sortSceneElements();
    void exportToJson();
};

#endif // ELEMENT_CREATOR_HPP
