#include "TabManager.hpp"

TabManager::TabManager(Vector2 pos, float width, float height)
    : position(pos), tabWidth(120), tabHeight(30), selectedTabIndex(0) {
    tabField = {position.x, position.y, width, tabHeight};
    contentField = {position.x, position.y + tabHeight, width, height - tabHeight};

    tabs.emplace_back(std::make_unique<HomeTab>(position.x, position.y, tabWidth, tabHeight));
    tabs.emplace_back(std::make_unique<AddTab>(position.x + tabWidth, position.y, tabWidth, tabHeight));
    repositionTabs();
}

void TabManager::repositionTabs() {
    float x = position.x;
    for (size_t i = 0; i < tabs.size(); ++i) {
        tabs[i]->setPosition(x, position.y);
        x += tabWidth + 5;
    }
}

void TabManager::draw() {
    for (size_t i = 0; i < tabs.size(); ++i) {
        tabs[i]->setActive(i == selectedTabIndex);
        tabs[i]->drawTab(tabField);
    }

    tabs[selectedTabIndex]->drawContent(contentField);
}

void TabManager::handleClick(Vector2 mousePos) {
    for (size_t i = 0; i < tabs.size(); ++i) {
        if (tabs[i]->checkClick(mousePos)) {
            if (tabs[i]->isAddButton()) {
                addContentTab("New Tab", nullptr); // временный null UI
            } else {
                selectedTabIndex = static_cast<int>(i);
            }
            return;
        }
    }
}

void TabManager::addContentTab(const std::string& name, BasicUI* ui) {
    tabs.insert(tabs.end() - 1,
                std::make_unique<ContentTab>(name, ui, 0, 0, tabWidth, tabHeight));
    selectedTabIndex = static_cast<int>(tabs.size() - 2);
    repositionTabs();
}
