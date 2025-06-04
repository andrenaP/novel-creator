#ifndef CONTEXT_MENU_HPP
#define CONTEXT_MENU_HPP

#include <string>
#include <vector>

#include "raylib.h"

#include "enums/Editors.hpp"


class ContextMenu 
{
private:
    std::vector<std::pair<std::string, Editors>> options;
    Vector2 position;
    float width;
    float height;
    bool isVisible;
    Rectangle bounds;
    int selectedOption;

    void updateBounds();

public:
    ContextMenu(Vector2 pos, float w, float h);
    
    void draw();
    
    void show();
    void hide();

    bool isMenuVisible() const;
    Editors handleClick(Vector2 mousePos);
};

#endif // CONTEXT_MENU_HPP