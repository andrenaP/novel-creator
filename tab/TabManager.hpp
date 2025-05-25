#ifndef TAB_MANAGER_HPP
#define TAB_MANAGER_HPP

#include <raylib.h>
// #include <vector>
// #include <string>

#include "Tab.hpp"


class TabManager
{
private:
    std::vector<Tab> tabs;
    int selectedTabIndex;
    int tabCounter;
    float tabWidth;
    float tabHeight;
    Vector2 position;
    Rectangle addButton;
    Rectangle canvas;

    void repositionTabs() 
    {
        float xPos = position.x;
        for (auto& tab : tabs)
        {
            tab.setPosition(xPos, position.y);
            // xPos += tabWidth - 20; // Overlap tabs slightly
            xPos += tabWidth + 5; // Overlap tabs slightly
        }
        addButton.x = xPos;
        addButton.y = position.y;
    }

public:
    TabManager(
        float x, float y, 
        float width, float height) 
    : 
        selectedTabIndex(-1), 
        tabWidth(width), tabHeight(height), 
        position({x, y}), tabCounter(1) 
    {
        addButton = {x, y, 30, height};
    }

    void addTab(const std::string& name = "") 
    {
        std::string tabName = name.empty() ? "Tab " + std::to_string(tabCounter++) : name;
        tabs.emplace_back(tabName, position.x, position.y, tabWidth, tabHeight);
        repositionTabs();
        if (selectedTabIndex == -1) 
        {
            selectedTabIndex = 0;
            tabs[0].setActive(true);
        }
    }

    void removeTab(int index) 
    {
        if (index >= 1 && index < tabs.size()) 
        {
            tabs.erase(tabs.begin() + index);
            if (selectedTabIndex == index) 
            {
                selectedTabIndex = -1;
                if (!tabs.empty()) 
                {
                    selectedTabIndex = std::min(index, static_cast<int>(tabs.size()) - 1);
                    if (selectedTabIndex >= 0) 
                    {
                        tabs[selectedTabIndex].setActive(true);
                    }
                }
            } else if (selectedTabIndex > index) 
            {
                selectedTabIndex--;
            }
            repositionTabs();
        }
    }

    void update() 
    {
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
        {
            // Check add button
            if (CheckCollisionPointRec(mousePos, addButton)) 
            {
                addTab();
                return;
            }

            // Check tabs
            for (size_t i = 0; i < tabs.size(); i++) 
            {
                // Check close button
                if (tabs[i].checkCloseClick(mousePos)) 
                {
                    removeTab(i);
                    return;
                }
                // Check tab click
                if (tabs[i].checkClick(mousePos)) 
                {
                    if (selectedTabIndex != -1) 
                    {
                        tabs[selectedTabIndex].setActive(false);
                    }
                    selectedTabIndex = i;
                    tabs[i].setActive(true);
                    break;
                }
            }
        }
    }

    void draw() 
    {
        
        // Draw tabs
        for (auto& tab : tabs) 
        {
            // Переделать этот хардкод. Я этим займусь позже
            tab.draw({10, 50, 780, 545});
        }
        // Draw add button
        DrawRectangleRounded(addButton, 0.05f, 20, LIGHTGRAY);
        DrawText("+", addButton.x + 10, addButton.y + 8, 20, BLACK);
    }

    int getSelectedTabIndex() const { return selectedTabIndex; }
};

#endif // TAB_MANAGER_HPP