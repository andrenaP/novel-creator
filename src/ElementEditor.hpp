#ifndef ELEMENT_EDITOR_HPP
#define ELEMENT_EDITOR_HPP

#include "raylib.h"
#include "raygui.h"

#include "Types.hpp"
#include "BasicUI.hpp"

#include <vector>
#include <string>

class ElementEditor : BasicUI
{
public:
    ElementEditor();
    ~ElementEditor();
    void update();
    void draw();
    std::vector<Element>& getElements();
    std::vector<Scene>& getScenes();

private:
    std::vector<Element> elements;
    std::vector<Scene> scenes; // Shared with SceneEditor
    int currentElementIndex;
    char nameBuffer[256];
    char textBuffer[1024];
    char charNameBuffer[256];
    char imageNameBuffer[256];
    char imagePathBuffer[256];
    char bgPathBuffer[256];
    int elementTypeIndex;
    bool showAddImage;
    bool showEditImage;
    int editImageIndex;
    int focusedTextBox;
    bool isEditing;
    float elementScrollOffset;
    float imageScrollOffset;

    void updateElementMode();
    void clearBuffers();
    void drawElementMode();
    void loadElementToUI();
    void saveElement();
    void exportToJson();
};

#endif // ELEMENT_EDITOR_HPP
