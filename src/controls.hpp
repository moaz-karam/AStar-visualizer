#ifndef CONTROLS_H
#define CONTROLS_H

#include "../include/raylib.h"
#include "../include/raygui.h"

#define NORMAL 0
#define HOVER 1
#define PRESSED 2

#define FONT_SIZE 30.0
#define FONT_SPACING 1

class Button
{
private:
    Rectangle rect;
    const char* text; 
    Color color;
    int state;
    float fontSize;

    bool isUnderMouse(Vector2 mouse)
    {
        return mouse.x > rect.x && mouse.y > rect.y
            && mouse.x < rect.x + rect.width
            && mouse.y < rect.y + rect.height;
    }


    Color getColor(bool isSelected)
    {
        if (isSelected) return ColorBrightness(color, -0.25);
        if (state == NORMAL) return color;
        else if (state == HOVER) return ColorBrightness(color, 0.3);
        else return ColorBrightness(color, -0.3);
    }

    void drawButton(bool isSelected)
    {
        DrawRectangleRec(rect, getColor(isSelected));
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, FONT_SPACING);
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

    Button()
    {
        this->fontSize = FONT_SIZE;
    }
    Button(Rectangle rect, const char* text, Color color, float fontSize = FONT_SIZE)
    {
        this->rect = rect;
        this->text = text;
        this->color = color;
        this->fontSize = fontSize;
        state = NORMAL;
    }

    // must be called before drawing the button
    bool updateState(Vector2 mouse, bool isPressed, bool isSelected)
    {
        if (isUnderMouse(mouse))
        {
            state = HOVER;
            if (isPressed) state = PRESSED;
        }
        else state = NORMAL;

        drawButton(isSelected);
        return isButtonPressed();
    }
    void setRect(float x, float y, float w, float h)
    {
        rect = (Rectangle){.x = x, .y = y, .width = w, .height = h};
    }
    void setRect(Rectangle rect)
    {
        this->rect = rect;
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

    void setFontSize(float fontSize)
    {
        this->fontSize = fontSize;
    }

};





#endif