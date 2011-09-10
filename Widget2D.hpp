#ifndef WIDGET2D_H_
#define WIDGET2D_H_

#include "Widget.hpp"

class Widget2D : public Widget
{
public:
    int GetY() { return myY; }
    void SetY(int y) { myY = y; }

private:
    int myY;
};

#endif // WIDGET2D_H_
