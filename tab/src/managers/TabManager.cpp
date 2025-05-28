#include "managers/TabManager.hpp"
#include <raylib.h>

TabManager::TabManager(
    Vector2 pos, 
    float width, 
    float height
): 
    position(pos),
    tabWidth(120),
    tabHeight(30),
    selectedTabIndex(0),
    contextMenu({{pos.x, pos.y + tabHeight}, 150, 30}) 
{
    this->tabField = {
        this->position.x, 
        this->position.y, 
        width, 
        tabHeight
    };
    this->contentField = {
        this->position.x, 
        this->position.y + tabHeight, 
        width, 
        height - tabHeight
    };

    this->tabs.emplace_back(
        std::make_unique<HomeTab>(
            this->position.x, 
            this->position.y, 
            tabWidth, 
            tabHeight
        )
    );
    this->tabs.emplace_back(
        std::make_unique<AddTab>(
            this->position.x + tabWidth, 
            this->position.y, 
            tabHeight
        )
    );
    repositionTabs();
}

void TabManager::repositionTabs()
{
    float x = position.x;
    for (size_t i = 0; i < this->tabs.size(); ++i) 
    {
        this->tabs[i]->setPosition(x, position.y);
        x += tabWidth + 5;
    }
}

void TabManager::draw() 
{
    for (size_t i = 0; i < tabs.size(); ++i) 
    {
        tabs[i]->setActive(i == selectedTabIndex);
        tabs[i]->drawTab(tabField);
    }

    tabs[selectedTabIndex]->drawContent(contentField);

    // Draw context menu if visible
    contextMenu.draw();
}

void TabManager::handleClick(Vector2 mousePos) 
{
    // Check if context menu is visible and handle its clicks
    if (contextMenu.isMenuVisible()) 
    {
        Editors selectedEditor = contextMenu.handleClick(mousePos);
        if (selectedEditor != Editors::NONE) 
        {
            std::string tabName;
            // BasicUI* ui;
            switch (selectedEditor) 
            {
                case Editors::ELEMENT: 
                    tabName = "Element Editor"; 
                    break;
                case Editors::NODE: 
                    tabName = "Node Editor"; 
                    break;
                case Editors::SCENE: 
                    tabName = "Scene Editor"; 
                    break;
                default: 
                    tabName = "New Tab"; 
                    break;
            }
            addContentTab(tabName, nullptr, selectedEditor); // Pass nullptr UI for now
        }
        return;
    }

    // Check tab clicks
    for (size_t i = 0; i < tabs.size(); ++i) 
    {
        if (tabs[i]->checkClick(mousePos)) 
        {
            if (tabs[i]->isAddButton()) 
            {
                contextMenu.show(); // Show context menu when AddTab is clicked
            } else 
            {
                selectedTabIndex = static_cast<int>(i);
            }
            return;
        }
    }
}

void TabManager::addContentTab(const std::string& name, BasicUI* ui, Editors editorType) 
{
    tabs.insert(
        tabs.end() - 1,
        std::make_unique<ContentTab>(name, ui, 0, 0, tabWidth, tabHeight)
    );
    selectedTabIndex = static_cast<int>(tabs.size() - 2);
    repositionTabs();
}