#ifndef TAB_HPP
#define TAB_HPP

#include <raylib.h>
#include <vector>
#include <string>


class Tab 
{
private:
    std::string name;
    Rectangle bounds;
    Rectangle closeButton;
    bool isActive;


public:
    Tab(const std::string& tabName, 
        float x, float y, 
        float width, float height) 
    : 
        name(tabName), 
        isActive(false) 
    {
        bounds = {x, y, width, height};
        closeButton = {x + width - 20, y + 5, 15, 15};
    }

    void draw(Rectangle pageCanvas) 
    {
        // Разделить на drawPage и drawContent. Позже займусь
        // Draw tab background with rounded corners
        Color bgColor = isActive ? WHITE : LIGHTGRAY;
        DrawRectangleRounded(bounds, 0.3f, 10, bgColor);
        
        // Draw tab name
        int fontSize = 16;
        std::string displayName = name.length() > 15 ? name.substr(0, 12) + "..." : name;
        int textWidth = MeasureText(displayName.c_str(), fontSize);
        float textX = bounds.x + 10;
        float textY = bounds.y + (bounds.height - fontSize) / 2;
        DrawText(displayName.c_str(), textX, textY, fontSize, BLACK);
        
        // Draw close button
        // if (isActive && this->name != "Home") 
        if (this->name != "Home") 
        {
            // DrawRectangleRec(closeButton, RED);
            DrawRectangleRounded(closeButton, 0.05f, 20, bgColor);
            DrawText("x", closeButton.x + 4, closeButton.y + 2, 12, WHITE);
        }

        DrawRectangleRoundedLines(pageCanvas, 0.05f, 20, DARKGRAY);

        // Draw outline
        // DrawRectangleRoundedLines(bounds, 0.3f, 10, DARKGRAY);
    }

    bool checkClick(Vector2 mousePos) 
    {
        return CheckCollisionPointRec(mousePos, bounds);
    }

    bool checkCloseClick(Vector2 mousePos) 
    {
        return CheckCollisionPointRec(mousePos, closeButton);
    }

    void setActive(bool active) { isActive = active; }
    Rectangle getBounds() const { return bounds; }
    void setPosition(float x, float y) 
    { 
        bounds.x = x; 
        bounds.y = y; 
        closeButton.x = x + bounds.width - 20;
        closeButton.y = y + 5;
    }
};

#endif