#ifndef TAB_MANAGER_HPP
#define TAB_MANAGER_HPP

#include <vector>
#include <memory>

#include "raylib.h"

#include "tabs/BasicTab.hpp"
#include "tabs/HomeTab.hpp"
#include "tabs/AddTab.hpp"
#include "tabs/ContentTab.hpp"

#include "tabs/ContextMenu.hpp"

#include "enums/Editors.hpp"

#include "editors/ElementEditor.hpp"
#include "editors/NodeManager.hpp"
#include "editors/SceneEditor.hpp"

#define BORDER 10.f
#define TAB_WIDTH 120.f
#define TAB_HEIGHT 30.f

class TabManager 
{
private:
int selectedTabIndex;
    std::vector<std::unique_ptr<BasicTab>> tabs;
    ContextMenu contextMenu;
    
    Rectangle tabField;
    Rectangle contentField;
    
    Vector2 position;
    float tabWidth;
    float tabHeight;
    float border;


    void repositionTabs();
    void updateResolution();

    Vector2 getScreenResolution();

public:
    TabManager(Vector2 pos);
    TabManager(Vector2 pos, float width, float height);

    void draw();
    void handleClick(Vector2 mousePos);
    // void addContentTab(const std::string& name, BasicUI* ui, Editors editorType);
    void addContentTab(const std::string& name, BasicUI* ui);
};

#endif // TAB_MANAGER_HPP