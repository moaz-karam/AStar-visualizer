#ifndef CONTROLS_H
#define CONTROLS_H

#include "../include/raylib.h"
#include "../include/raygui.h"

class Button
{

private:
    Rectangle rect;
    const char* text; 
    Color color;

public:

    void setRect(float x, float y, float w, float h)
    {
        rect = (Rectangle){.x = x, .y = y, .width = w, .height = h};
    }
    void setText(const char* t)
    {
        text = t;
    }
    void setColor(Color c)
    {
        color = c;
    }

    Rectangle getRect()
    {
        return rect;
    }
    void getText(const char* t)
    {
        t = text;
    }
    Color getColor()
    {
        return color;
    }

};





#endif