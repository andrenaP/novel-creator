#ifndef ADD_TAB_HPP
#define ADD_TAB_HPP

#include "BasicTab.hpp"

class AddTab : public BasicTab 
{
public:
    AddTab(
        float x, float y, 
        float size
    ): 
        BasicTab("+", x, y, size, size) 
    {}

    void drawTab(Rectangle tabField) override 
    {
        DrawRectangleRounded(bounds, 0.3f, 10, LIGHTGRAY);
        DrawText("+", bounds.x + 10, bounds.y + 5, 20, BLACK);
    }

    void drawContent(Rectangle contentField) override 
    {
        // Не отображает контент
    }

    bool isClosable() const override { return false; }
    bool isAddButton() const override { return true; }
};

#endif // ADD_TAB_HPP
