#ifndef CONTENT_TAB_HPP
#define CONTENT_TAB_HPP

#include "BasicTab.hpp"
#include "ui/BasicUI.hpp"

class ContentTab : public BasicTab 
{
private:
    BasicUI* ui;

public:
    ContentTab(
        const std::string& tabName, 
        BasicUI* uiComponent,
        float x, float y, 
        float width, float height
    ): 
        BasicTab(tabName, x, y, width, height), 
        ui(uiComponent) 
    {}

    void drawTab(Rectangle tabField) override 
    {
        Color bgColor = isActive ? WHITE : LIGHTGRAY;
        DrawRectangleRounded(bounds, 0.3f, 10, bgColor);
        
        std::string label = name.length() > 15 ? name.substr(0, 12) + "..." : name;
        DrawText(label.c_str(), bounds.x + 10, bounds.y + 5, 16, BLACK);
    }

    void drawContent(Rectangle contentField) override 
    {
        if (ui) 
            ui->drawClipped(contentField);
    }
};

#endif // CONTENT_TAB_HPP
