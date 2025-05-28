#ifndef TAB_MANAGER_HPP
#define TAB_MANAGER_HPP

#include <vector>
#include <memory>

#include "tabs/BasicTab.hpp"
#include "tabs/HomeTab.hpp"
#include "tabs/AddTab.hpp"
#include "tabs/ContentTab.hpp"

class TabManager {
private:
    std::vector<std::unique_ptr<BasicTab>> tabs;
    int selectedTabIndex;
    float tabWidth;
    float tabHeight;
    Vector2 position;
    Rectangle tabField;
    Rectangle contentField;

    void repositionTabs();

public:
    TabManager(Vector2 pos, float width, float height);
    void draw();
    void handleClick(Vector2 mousePos);
    void addContentTab(const std::string& name, BasicUI* ui);
};

#endif // TAB_MANAGER_HPP
