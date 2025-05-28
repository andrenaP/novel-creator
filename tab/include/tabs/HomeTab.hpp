#ifndef HOME_TAB_HPP
#define HOME_TAB_HPP

#include "BasicTab.hpp"

class HomeTab : public BasicTab {
public:
    HomeTab(
        float x, float y, 
        float width, float height
    ): 
        BasicTab("Home", x, y, width, height) 
    {}

    void drawTab(Rectangle tabField) override 
    {
        DrawRectangleRounded(bounds, 0.3f, 10, isActive ? WHITE : LIGHTGRAY);
        DrawText("Home", bounds.x + 10, bounds.y + 5, 16, BLACK);
    }

    void drawContent(Rectangle contentField) override 
    {
        DrawText("Welcome to Home Tab", contentField.x + 20, contentField.y + 20, 20, GRAY);
    }

    bool isClosable() const override { return false; }
};

#endif // HOME_TAB_HPP
