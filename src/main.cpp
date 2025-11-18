

#include <raylib.h>
#include <raygui.h>
#include "../algorithms/searcher.hpp"

#define FRAMES 60.0f

int main()
{

    InitWindow(800, 800, "Visualizer");

    Searcher searcher(Vector2{.x = 0, .y = 0}, Vector2{.x = 800, .y = 800});

    Hashtable<int, CellType>::HashIterator iter;

    SetTargetFPS(120);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(WHITE);

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
        else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
        {
            searcher.startDragging();
        }
        else if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON))
        {
            searcher.stopDragging();
        }


        if (searcher.isDragging())
        {
            Vector2 diff = GetMouseDelta();
            searcher.drag(diff.x, diff.y);
        }

        // update the searcher and start the iterator
        searcher.update(iter);

        Rectangle rect;
        // draw the cells with the help of the iterator
        for (iter; iter.hasNext(); iter.next())
        {
            searcher.generateRect(iter.getKey(), &rect);
            DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLORS[iter.getValue()]);
        }

        // draw the current cell with red
        if (!searcher.isPathFound())
        {
            searcher.generateRect(searcher.getCurrentKey(), &rect);
            DrawRectangle(rect.x, rect.y, rect.width, rect.height, RED);
        }



        EndDrawing();
    }

    CloseWindow();

    return 0;
}