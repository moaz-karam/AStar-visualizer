#ifndef CONTROLS_H
#define CONTROLS_H

#include "../include/raylib.h"
#include "../include/raygui.h"

#define NORMAL 0
#define HOVER 1
#define PRESSED 2


class Button
{
private:
    Rectangle rect;
    const char* text; 
    Color color;
    int state;

    const int fontSize = 20;
    const int fontSpacing = 1;

    bool isUnderMouse(Vector2 mouse)
    {
        return mouse.x > rect.x && mouse.y > rect.y
            && mouse.x < rect.x + rect.width
            && mouse.y < rect.y + rect.height;
    }


    Color getColor()
    {
        if (state == NORMAL) return color;
        else if (state == HOVER) return ColorBrightness(color, 0.3);
        else return ColorBrightness(color, -0.3);
    }

    void drawButton()
    {
        DrawRectangleRec(rect, getColor());
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSpacing);
        Vector2 textPos;
        textPos.x = rect.x + (rect.width - textSize.x) / 2;
        textPos.y = rect.y + (rect.height - textSize.y) / 2;

        DrawText(text, textPos.x, textPos.y, fontSize, BLACK);
    }

    bool isButtonPressed()
    {
        return state == PRESSED;
    }

public:

    Button(Rectangle rect, const char* text, Color color)
    {
        this->rect = rect;
        this->text = text;
        this->color = color;
        state = NORMAL;
    }

    // must be called before drawing the button
    bool updateState(Vector2 mouse, bool isPressed)
    {
        if (isUnderMouse(mouse))
        {
            state = HOVER;
            if (isPressed) state = PRESSED;
        }
        else{state = NORMAL;}

        drawButton();
        return isButtonPressed();
    }
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

};





#endif