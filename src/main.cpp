

#include "../include/raylib.h"
#include "../include/raygui.h"

#include "../algorithms/searcher.hpp"

#define FRAMES 60.0f

int main()
{

    InitWindow(800, 800, "Visualizer");

    Searcher searcher(Vector2{.x = 200, .y = 200}, Vector2{.x = 400, .y = 400});

    Hashtable<Vector2I, CellType>::HashIterator iter;

    SetTargetFPS(120);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(WHITE);


        DrawRectangle(200, 200, 400, 400, BLUE);

        // select the type
        if (IsKeyPressed(KEY_S))
        {
            searcher.select(SOURCE);
        }
        else if (IsKeyPressed(KEY_W))
        {
            searcher.select(WALL);
        }
        else if (IsKeyPressed(KEY_T))
        {
            searcher.select(TARGET);
        }
        else if (IsKeyPressed(KEY_R))
        {
            searcher.select(REMOVE);
        }
        else if (IsKeyPressed(KEY_C))
        {
            searcher.run();
        }
        else if (IsKeyPressed(KEY_X))
        {
            searcher.pause();
        }

        // add particles if mouse is pressed
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            searcher.press(GetMousePosition());
        }
        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            Vector2 diff = GetMouseDelta();
            searcher.drag(diff.x, diff.y);
        }
        
        searcher.zoom((int)GetMouseWheelMove());

        // update the searcher and start the iterator
        searcher.update(iter);

        Rectangle rect;
        // draw the cells with the help of the iterator
        for (iter; iter.hasNext(); iter.next())
        {
            searcher.generateRect(iter.getKey(), &rect);
            if (searcher.isInRange(rect))
            {
                DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLORS[iter.getValue()]);
            }
        }

        // draw the current cell with red
        if (!searcher.isPathFound())
        {
            searcher.generateRect(searcher.getCurrentPos(), &rect);
            if (searcher.isInRange(rect))
            {
                DrawRectangle(rect.x, rect.y, rect.width, rect.height, RED);
            }
        }

        Vector2 sPoint;
        Vector2 ePoint;

        for (int col = 1; searcher.isColumn(col); col += 1)
        {
            searcher.getColumn(col, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, ColorAlpha(BLACK, 1));
        }

        for (int row = 1; searcher.isRow(row); row += 1)
        {
            searcher.getRow(row, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, ColorAlpha(BLACK, 1));
        }


        EndDrawing();
    }

    CloseWindow();

    return 0;
}