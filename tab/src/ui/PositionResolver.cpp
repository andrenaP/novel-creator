#include "ui/PositionResolver.hpp"


PositionResolver::PositionResolver(Rectangle rect): 
    _rect(rect)
{}
PositionResolver::PositionResolver():
    _rect({0, 0, 100, 200}) 
{}

PositionResolver::~PositionResolver() 
{}

Rectangle PositionResolver::resolve(const Rectangle& rect) const
{
    return {
        this->_rect.x + rect.x,
        this->_rect.y + rect.y,
        rect.width,
        rect.height
    };
}
Vector2 PositionResolver::resolve(Vector2 pos) const
{
    return {
        this->_rect.x + pos.x,
        this->_rect.y + pos.y
    };
}

int PositionResolver::resolveX(int x) const 
{ 
    return static_cast<int>(this->_rect.x) + x; 
}
int PositionResolver::resolveY(int y) const 
{ 
    return static_cast<int>(this->_rect.y) + y; 
}

void PositionResolver::beginClip() const 
{
    BeginScissorMode(this->_rect.x, this->_rect.y, this->_rect.width, this->_rect.height);
}
void PositionResolver::endClip() const 
{
    EndScissorMode();
}

Rectangle PositionResolver::getRect() const 
{ 
    return this->_rect; 
}

PositionResolver PositionResolver::nested(Rectangle local) const 
{
    return PositionResolver(resolve(local));
}
