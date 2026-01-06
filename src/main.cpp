#include <time.h>
#include "../include/raylib.h"
#include "../include/raygui.h"

#include "./searchers.hpp"
#include "./controls.hpp"

#define FRAMES 60.0f
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 60
#define SCREEN_PARTS 10.0f
#define MENU_GAP 10.0f
#define CONTROL_BUTTONS_NUMBER 7
#define ALGORITHM_BUTTONS_NUMBER 3

#define CONTROLS_CONTROL 0
#define START_CONTROL 1
#define CLEAR_CONTROL 2
#define SOURCE_CONTROL 3
#define TARGET_CONTROL 4
#define WALL_CONTROL 5
#define REMOVE_CONTROL 6


#define ALGORITHMS 0
#define DIJKSTRA 1
#define ASTAR 2


float screenWidth = 0;
float screenHeight = 0;


int searcherType = DIJKSTRA;
Searcher* searcher;
Hashtable<Vector2I, Cell>::HashIterator iter;

Button controlButtons[CONTROL_BUTTONS_NUMBER];
const char* controlButtonsText[] = {"CONTROLS: ", "START", "CLEAR", "SOURCE", "TARGET", "WALL", "REMOVE"};
const Color controlButtonsColor[] = {WHITE, GREEN, LIGHTGRAY, ORANGE, DARKBLUE, BROWN, RED};

Button algorithmButtons[ALGORITHM_BUTTONS_NUMBER];
const char* algorithmButtonsText[] = {"ALGORITHMS: ", "DIJKSTRA", "ASTAR"};
const Color algorithmButtonsColor[] = {WHITE, PURPLE, YELLOW};

int currentControl;
int currentAlgorithm;

void selectSearcherType(int type)
{
    if (searcherType == type) return;
    searcherType = type;
    if (searcherType == DIJKSTRA)
    {
        Searcher* oldSearcher = searcher;
        searcher = new Dijkstra(oldSearcher);
        delete oldSearcher;
    }
    else if (searcherType == ASTAR)
    {
        Searcher* oldSearcher = searcher;
        searcher = new AStar(oldSearcher);
        delete oldSearcher;
    }
}


void updateButtons(Vector2 mouse, bool isPressed)
{
    // drawing titles
    controlButtons[CONTROLS_CONTROL].updateState(mouse, false, false);
    algorithmButtons[ALGORITHMS].updateState(mouse, false, false);

    // drawing buttons
    if (controlButtons[START_CONTROL].updateState(mouse, isPressed, currentControl == START_CONTROL))
    {
        searcher->run();
        currentControl = START_CONTROL;
    }
    if (controlButtons[CLEAR_CONTROL].updateState(mouse, isPressed, false))
    {
        searcher->clear();
    }
    if (controlButtons[SOURCE_CONTROL].updateState(mouse, isPressed, currentControl == SOURCE_CONTROL))
    {
        searcher->select(SOURCE);
        currentControl = SOURCE_CONTROL;
    }
    if (controlButtons[TARGET_CONTROL].updateState(mouse, isPressed, currentControl == TARGET_CONTROL))
    {
        searcher->select(TARGET);
        currentControl = TARGET_CONTROL;
    }
    if (controlButtons[WALL_CONTROL].updateState(mouse, isPressed, currentControl == WALL_CONTROL))
    {
        searcher->select(WALL);
        currentControl = WALL_CONTROL;
    }
    if (controlButtons[REMOVE_CONTROL].updateState(mouse, isPressed, currentControl == REMOVE_CONTROL))
    {
        searcher->select(REMOVE);
        currentControl = REMOVE_CONTROL;
    }
    if (algorithmButtons[DIJKSTRA].updateState(mouse, isPressed, currentAlgorithm == DIJKSTRA))
    {
        selectSearcherType(DIJKSTRA);
        currentAlgorithm = DIJKSTRA;
    }
    if (algorithmButtons[ASTAR].updateState(mouse, isPressed, currentAlgorithm == ASTAR))
    {
        selectSearcherType(ASTAR);
        currentAlgorithm = ASTAR;
    }
}

void initButtons()
{
    currentControl = WALL_CONTROL;
    currentAlgorithm = DIJKSTRA;

    // control buttons initialization
    for (int i = 0; i < CONTROL_BUTTONS_NUMBER; i += 1)
    {
        Rectangle rect = {
            .x = i * (BUTTON_WIDTH + MENU_GAP) + MENU_GAP,
            .y = MENU_GAP,
            .width = BUTTON_WIDTH,
            .height = BUTTON_HEIGHT
        };
        controlButtons[i].setColor(controlButtonsColor[i]);
        controlButtons[i].setText(controlButtonsText[i]);
        controlButtons[i].setRect(rect);
    }

    for (int i = 0; i < ALGORITHM_BUTTONS_NUMBER; i += 1)
    {
        Rectangle rect = {
            .x = i * (BUTTON_WIDTH + MENU_GAP) + MENU_GAP,
            .y = 2 * MENU_GAP + BUTTON_HEIGHT,
            .width = BUTTON_WIDTH,
            .height = BUTTON_HEIGHT
        };
        algorithmButtons[i].setColor(algorithmButtonsColor[i]);
        algorithmButtons[i].setText(algorithmButtonsText[i]);
        algorithmButtons[i].setRect(rect);
    }

}

int main()
{

    InitWindow(screenWidth, screenHeight, "Visualizer");
    ToggleFullscreen();
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    searcher = new Dijkstra(Vector2{.x = 0, .y = screenHeight / (SCREEN_PARTS)},
         Vector2{.x = screenWidth, .y = (SCREEN_PARTS - 1) * screenHeight / (SCREEN_PARTS)});

    initButtons();

    clock_t st = clock();

    // initializing buttons
    while (!WindowShouldClose())
    {
        clock_t now = clock();
        if ((float)(now - st) / CLOCKS_PER_SEC >= 1.0f / FRAMES)
        {
            st = now;
            continue;
        }
        BeginDrawing();

        ClearBackground(WHITE);


        DrawRectangle(0, screenHeight / SCREEN_PARTS, screenWidth, (SCREEN_PARTS - 1) * screenHeight / (SCREEN_PARTS), LIGHTGRAY);


        // managing buttons
        Vector2 mouse = GetMousePosition();
        bool isPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        updateButtons(mouse, isPressed);



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
        searcher->zoom((int)GetMouseWheelMove());

        // update the searcher and start the iterator
        searcher->update(iter);

        Rectangle rect;
        // draw the cells with the help of the iterator
        for (iter; iter.hasNext(); iter.next())
        {
            if (searcher->isValidRect(iter.getKey()))
            {
                searcher->generateRect(iter.getKey(), &rect, &iter.getValue());
                DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLORS[iter.getValue().ct]);
            }
        }

        Vector2 sPoint;
        Vector2 ePoint;

        Color lineColor = ColorAlpha(BLACK, 0.2);

        for (int col = 1; searcher->isColumn(col); col += 1)
        {
            searcher->getColumn(col, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, lineColor);
        }

        for (int row = 1; searcher->isRow(row); row += 1)
        {
            searcher->getRow(row, &sPoint, &ePoint);
            DrawLine(sPoint.x, sPoint.y, ePoint.x, ePoint.y, lineColor);
        }


        EndDrawing();
    }

    delete searcher;
    CloseWindow();

    return 0;
}