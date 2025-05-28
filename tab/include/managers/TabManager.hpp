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


class TabManager 
{
private:
    std::vector<std::unique_ptr<BasicTab>> tabs;
    int selectedTabIndex;
    float tabWidth;
    float tabHeight;
    Vector2 position;
    Rectangle tabField;
    Rectangle contentField;
    ContextMenu contextMenu;

    void repositionTabs();

public:
    TabManager(Vector2 pos, float width, float height);
    void draw();
    void handleClick(Vector2 mousePos);
    void addContentTab(const std::string& name, BasicUI* ui, Editors editorType);
};

#endif // TAB_MANAGER_HPP