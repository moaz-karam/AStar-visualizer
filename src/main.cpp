

#include <raylib.h>
#include <raygui.h>

#include "../algorithms/searcher.hpp"

int main()
{

    InitWindow(800, 800, "Visualizer");

    Searcher searcher(Vector2{.x = 0, .y = 0}, Vector2{.x = 800, .y = 800});

    Hashtable<int, Cell>::HashIterator iter;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        // add particles if mouse is pressed
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            searcher.press(GetMousePosition());
        }

        // update the searcher and start the iterator
        searcher.update(iter);

        // draw the cells with the help of the iterator
        for (iter; iter.hasNext(); iter.next())
        {
            Cell cc = iter.getData();
            DrawRectangle(cc.rect.x, cc.rect.y, cc.rect.width, cc.rect.height, cc.color);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}