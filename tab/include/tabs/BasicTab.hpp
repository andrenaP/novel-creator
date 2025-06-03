#ifndef BASIC_TAB_HPP
#define BASIC_TAB_HPP

#include <raylib.h>
#include <string>

class BasicTab 
{
protected:
    std::string name;
    Rectangle bounds;
    bool isActive;

public:
    BasicTab(
        const std::string& name, 
        float x, float y, 
        float width, float height
    ): 
        name(name), 
        bounds{x, y, width, height}, 
        isActive(false) 
    {}

    virtual ~BasicTab() = default;

    virtual void drawTab(Rectangle tabField) = 0;
    virtual void drawContent(Rectangle contentField) = 0;

    virtual bool isClosable() const { return true; }
    virtual bool isAddButton() const { return false; }

    virtual bool checkClick(Vector2 mousePos) 
    {
        return CheckCollisionPointRec(mousePos, bounds);
    }

    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }

    void setPosition(float x, float y) 
    {
        this->bounds.x = x;
        this->bounds.y = y;
    }

    Rectangle getBounds() const { return bounds; }
    std::string getName() const { return name; }
};

#endif // BASIC_TAB_HPP
