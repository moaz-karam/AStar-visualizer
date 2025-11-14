#ifndef SEARCHER_H
#define SEARCHER_H

#include <raylib.h>
#include "../data_structures/hashtable.hpp"

#define CELL_WIDTH 5
#define CELL_HEIGHT 5


enum CellType
{
    SOURCE = 0, WALL = 1, CHECKED = 2, TARGET = 3
};

const Color COLORS[] = {YELLOW, BROWN, SKYBLUE, DARKBLUE};

struct Cell
{
    Rectangle rect;
    Color color;
    CellType type;
};

class Searcher
{
private:

    struct Vector2I
    {
        int x;
        int y;
    };

    struct Grid
    {
        Vector2I startingPoint;
        Vector2I cellsNumber;
        Hashtable<int, Cell> table;
    };

    bool running;
    CellType selectedType;
    Grid grid;

    // helper methods
    // ---------------------------------------------------------------------------------------------------------


    virtual bool isValid(int x, int y)
    {
        return x >= 0 && y >= 0 && x < grid.cellsNumber.x && y < grid.cellsNumber.y;
    }

    virtual Vector2I convertMouse(Vector2 mouse)
    {
        Vector2I v;
        v.x = ((int)(mouse.x) - grid.startingPoint.x) / CELL_WIDTH;
        v.y = ((int)(mouse.y) - grid.startingPoint.y) / CELL_HEIGHT;
        return v;
    }

    virtual int generateKey(Vector2I pos)
    {
        return pos.x + pos.y * grid.cellsNumber.x;
    }

    virtual Rectangle generateRect(Vector2I cellPos)
    {
        Rectangle rect;
        rect.width = CELL_WIDTH;
        rect.height = CELL_HEIGHT;

        rect.x = cellPos.x * CELL_WIDTH + grid.startingPoint.x;
        rect.y = cellPos.y * CELL_HEIGHT + grid.startingPoint.y;

        return rect;
    }

    virtual void putToGrid(Vector2I cellPos)
    {
        Cell c;
        c.rect = generateRect(cellPos);
        c.type = selectedType;
        c.color = COLORS[selectedType];

        grid.table.insert(generateKey(cellPos), c);
    }



    // ---------------------------------------------------------------------------------------------------------
    

public:
    Searcher(Vector2 startingPos, Vector2 dimensions)
    {
        grid.startingPoint.x = startingPos.x;
        grid.startingPoint.y = startingPos.y;


        grid.cellsNumber.x = (int)dimensions.x / (int)CELL_WIDTH;
        grid.cellsNumber.y = (int)dimensions.y / (int)CELL_HEIGHT;
        
        running = false;

        selectedType = WALL;

    }
    virtual void select(CellType ct) {selectedType = ct;}
    virtual void run() {running = true;}
    virtual void pause() {running = false;}

    virtual void press(Vector2 mouse)
    {
        if (running) {return;}
        putToGrid(convertMouse(mouse));
    }

    void update(Hashtable<int, Cell>::HashIterator& iter)
    {
        iter.begin(grid.table);
    }

};

#endif