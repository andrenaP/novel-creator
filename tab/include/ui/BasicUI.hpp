#ifndef BASIC_UI_HPP
#define BASIC_UI_HPP

#include "raylib.h"

#include "ui/PositionResolver.hpp"

class BasicUI
{
protected:
    PositionResolver _resolver;

public:
    BasicUI():
        _resolver()
    {}
    BasicUI(Rectangle rect):
        _resolver(rect)
    {}
    ~BasicUI() {}

    virtual void draw()   {}
    virtual void drawClipped(Rectangle contentField)   {}
    virtual void update() {}
};

#endif