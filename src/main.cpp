

#include "../include/raylib.h"
#include "../include/raygui.h"

#include "./searchers.hpp"
#include "./controls.hpp"
#define FRAMES 60.0f



Searcher* searcher = new AStar(Vector2{.x = 200, .y = 200}, Vector2{.x = 400, .y = 400});

Hashtable<Vector2I, CellType>::HashIterator iter;


Button startBtn({0, 0, 100, 60}, "START", GREEN);
Button sourceBtn({200, 0, 100, 60}, "SOURCE", ORANGE);
Button targetBtn({400, 0, 100, 60}, "TARGET", DARKBLUE);
Button wallBtn({600, 0, 100, 60}, "WALL", BROWN);
Button removeBtn({0, 100, 100, 60}, "REMOVE", GRAY);
Button pauseBtn({200, 100, 100, 60}, "PAUSE", RED);




void updateButtons(Vector2 mouse, bool isPressed)
{
    if (startBtn.updateState(mouse, isPressed))
    {
        searcher->run();
    }
    if (sourceBtn.updateState(mouse, isPressed))
    {
        searcher->select(SOURCE);
    }
    if (targetBtn.updateState(mouse, isPressed))
    {
        searcher->select(TARGET);
    }
    if (wallBtn.updateState(mouse, isPressed))
    {
        searcher->select(WALL);
    }
    if (removeBtn.updateState(mouse, isPressed))
    {
        searcher->select(REMOVE);
    }
    if (pauseBtn.updateState(mouse, isPressed))
    {
        searcher->pause();
    }
}

int main()
{

    InitWindow(800, 800, "Visualizer");
    SetTargetFPS(120);

    // initializing buttons
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(WHITE);


        DrawRectangle(200, 200, 400, 400, GRAY);


        // add particles if mouse is pressed
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            searcher->press(GetMousePosition());
        }
        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            Vector2 diff = GetMouseDelta();
            searcher->drag(diff.x, diff.y);
        }

        // managing buttons
        Vector2 mouse = GetMousePosition();
        bool isPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        updateButtons(mouse, isPressed);

        searcher->zoom((int)GetMouseWheelMove());

        // update the searcher and start the iterator
        searcher->update(iter);

        Rectangle rect;
        // draw the cells with the help of the iterator
        for (iter; iter.hasNext(); iter.next())
        {
            if (searcher->isValid(iter.getKey()))
            {
                searcher->generateRect(iter.getKey(), &rect);
                DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLORS[iter.getValue()]);
            }
        }

        // draw the current cell with red
        if (!searcher->isPathFound() && searcher->isRunning())
        {
            if (searcher->isValid(searcher->getCurrentPos()))
            {
                searcher->generateRect(searcher->getCurrentPos(), &rect);
                DrawRectangle(rect.x, rect.y, rect.width, rect.height, RED);
            }
        }

        Vector2 sPoint;
        Vector2 ePoint;

        for (int col = 1; searcher->isColumn(col); col += 1)
        {
            searcher->getColumn(col, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, ColorAlpha(BLACK, 0.2));
        }

        for (int row = 1; searcher->isRow(row); row += 1)
        {
            searcher->getRow(row, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, ColorAlpha(BLACK, 0.2));
        }


        EndDrawing();
    }

    delete searcher;
    CloseWindow();

    return 0;
}