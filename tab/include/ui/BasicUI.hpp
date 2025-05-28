#ifndef BASIC_UI_HPP
#define BASIC_UI_HPP

#include "raylib.h"

class BasicUI
{
private:
    /* data */
public:
    BasicUI()  {}
    ~BasicUI() {}

    virtual void draw()   {}
    virtual void draw(Rectangle& drawArea)   {}
    virtual void update() {}
};

#endif