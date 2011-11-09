#ifndef WIDGET_H_
#define WIDGET_H_

class Widget
{
public:
    int GetX() { return myX; }
    void SetX(int x) { myX = x; }

private:
    int myX;
};

#endif // WIDGET_H_
