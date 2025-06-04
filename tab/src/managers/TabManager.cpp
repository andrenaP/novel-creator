#include "managers/TabManager.hpp"

TabManager::TabManager(Vector2 pos): 
    position(pos),
    tabWidth(TAB_WIDTH),
    tabHeight(TAB_HEIGHT),
    selectedTabIndex(0),
    contextMenu({{pos.x, pos.y + tabHeight}, 150, 30}) 
{
    this->updateResolution();

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


TabManager::TabManager(
    Vector2 pos, 
    float width, 
    float height
): 
    position(pos),
    tabWidth(TAB_WIDTH),
    tabHeight(TAB_HEIGHT),
    selectedTabIndex(0),
    contextMenu({{pos.x, pos.y + tabHeight}, 150, 30}) 
{
    // this->tabField = {
    //     this->position.x, 
    //     this->position.y, 
    //     width, 
    //     tabHeight
    // };
    // this->contentField = {
    //     this->position.x, 
    //     this->position.y + tabHeight, 
    //     width, 
    //     height - tabHeight
    // };
    
    this->updateResolution();

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

void TabManager::updateResolution()
{
    Vector2 screenSize = this->getScreenResolution();

    this->border = position.x;

    this->tabField = {
        this->border,
        this->border,
        screenSize.x - this->border * 2,
        this->tabHeight
    };

    this->contentField = {
        this->border,
        this->tabField.height + this->border,
        screenSize.x - this->border * 2,
        screenSize.y - this->border * 2 - this->tabHeight
    };
}

Vector2 TabManager::getScreenResolution()
{
    return {
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
}



void TabManager::draw() 
{   
    DrawRectangleLinesEx(this->tabField, 2, GREEN);
    DrawRectangleLinesEx(this->contentField, 2, GREEN);
    
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
            void* ui;

            // TEMP
            std::vector<Scene>   sc {};
            std::vector<Element> el {};

            switch (selectedEditor) 
            {
                case Editors::ELEMENT: 
                    tabName = "Element Editor"; 
                    // ui = new ElementEditor();
                    ui = new ElementEditor(this->contentField);
                    break;
                case Editors::NODE: 
                    tabName = "Node Editor"; 
                    ui = new NodeManager(sc);
                    break;
                case Editors::SCENE: 
                    tabName = "Scene Editor"; 
                    ui = new SceneEditor(el, sc);
                    break;
                default: 
                    tabName = "New Tab"; 
                    break;
            }
            // addContentTab(tabName, nullptr, selectedEditor); // Pass nullptr UI for now
            addContentTab(tabName, (BasicUI*)ui);
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

// void TabManager::addContentTab(const std::string& name, BasicUI* ui, Editors editorType) 
void TabManager::addContentTab(const std::string& name, BasicUI* ui) 
{
    tabs.insert(
        tabs.end() - 1,
        std::make_unique<ContentTab>(name, ui, 0, 0, tabWidth, tabHeight)
    );
    selectedTabIndex = static_cast<int>(tabs.size() - 2);
    repositionTabs();
}