#ifndef ELEMENT_EDITOR_HPP
#define ELEMENT_EDITOR_HPP

#include "raylib.h"
#include "raygui.h"

#include "Types.hpp"
#include "ui/BasicUI.hpp"
#include "ui/PositionResolver.hpp"

#include <vector>
#include <string>

class ElementEditor : protected BasicUI
{
public:
    ElementEditor();
    ElementEditor(Rectangle contentField);
    ~ElementEditor();

    void update();
    void draw();
    void drawClipped(Rectangle contentField) override;
    
    std::vector<Element>& getElements();
    std::vector<Scene>& getScenes();

private:
    std::vector<Element> elements;
    std::vector<Scene> scenes; // Shared with SceneEditor

    int currentElementIndex;
    int elementTypeIndex;
    int editImageIndex;
    int focusedTextBox;
    char nameBuffer[256];
    char textBuffer[1024];
    char charNameBuffer[256];
    char imageNameBuffer[256];
    char imagePathBuffer[256];
    char bgPathBuffer[256];
    bool showAddImage;
    bool showEditImage;
    bool isEditing;
    float elementScrollOffset;

    void updateElementMode();
    void clearBuffers();
    void drawClippedElementMode();
    void drawElementMode();
    void loadElementToUI();
    void saveElement();
    void exportToJson();
};

#endif // ELEMENT_EDITOR_HPP
