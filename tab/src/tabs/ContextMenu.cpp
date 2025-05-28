#include "tabs/ContextMenu.hpp"
#include <raylib.h>

ContextMenu::ContextMenu(
    Vector2 pos, 
    float w, float h
): 
    position(pos), 
    width(w), height(h), 
    isVisible(false), 
    selectedOption(-1) 
{
    options = {
        {"Element Editor", Editors::ELEMENT},
        {"Node Editor", Editors::NODE},
        {"Scene Editor", Editors::SCENE}
    };
    updateBounds();
}

void ContextMenu::updateBounds() 
{
    bounds = {
        position.x, 
        position.y, 
        width, 
        height * options.size()
    };
}

void ContextMenu::draw() 
{
    if (!isVisible) 
        return;

    // Draw background
    DrawRectangleRec(bounds, LIGHTGRAY);

    // Draw options
    for (size_t i = 0; i < options.size(); ++i) 
    {
        Rectangle optionRect = {
            position.x, 
            position.y + i * height, 
            width, 
            height
        };
        Color bgColor = (selectedOption == static_cast<int>(i)) ? SKYBLUE : WHITE;
        
        DrawRectangleRec(optionRect, bgColor);
        DrawText(
            options[i].first.c_str(), 
            position.x + 10, 
            position.y + i * height + 10, 
            20, BLACK
        );
        DrawRectangleLinesEx(optionRect, 1, DARKGRAY);
    }
}
void ContextMenu::show() 
{
    isVisible = true;
}

void ContextMenu::hide() 
{
    isVisible = false;
    selectedOption = -1;
}

bool ContextMenu::isMenuVisible() const 
{
    return isVisible;
}

Editors ContextMenu::handleClick(Vector2 mousePos) 
{
    if (!isVisible || !CheckCollisionPointRec(mousePos, bounds)) 
    {
        hide();
        return Editors::NONE;
    }

    for (size_t i = 0; i < options.size(); ++i) 
    {
        Rectangle optionRect = {position.x, position.y + i * height, width, height};
        if (CheckCollisionPointRec(mousePos, optionRect)) 
        {
            selectedOption = static_cast<int>(i);
            hide();
            return options[i].second;
        }
    }
    return Editors::NONE;
}

