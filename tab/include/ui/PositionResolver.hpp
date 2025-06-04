#ifndef POSITION_RESOLVER_HPP
#define POSITION_RESOLVER_HPP

#include "raylib.h"

class PositionResolver
{
private:
    Rectangle _rect;

public:
    PositionResolver(Rectangle rect);
    PositionResolver();

    ~PositionResolver();

    Rectangle resolve(const Rectangle& rect) const;
    Vector2 resolve(Vector2 pos) const;

    int resolveX(int x) const;
    int resolveY(int y) const;

    void beginClip() const;
    void endClip() const;

    Rectangle getRect() const;
    PositionResolver nested(Rectangle local) const;
};

#endif // POSITION_RESOLVER_HPP
