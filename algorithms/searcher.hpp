#ifndef SEARCHER_H
#define SEARCHER_H

#include <raylib.h>
#include "../data_structures/hashtable.hpp"
#include "../data_structures/heap.hpp"

#define CELL_WIDTH 10
#define CELL_HEIGHT 10
#define MIN_X_EDGE -1
#define MIN_Y_EDGE -1
#define MAX_X_EDGE 1
#define MAX_Y_EDGE 1

enum CellType
{
    SOURCE = 0, CHECKED = 1, WALL = 2, TARGET = 3, PATH = 4, REMOVE
};

const Color COLORS[] = {DARKGREEN, SKYBLUE, BROWN, DARKBLUE, YELLOW};


struct Vector2I
{
    int x;
    int y;
};

class Searcher
{
private:

    struct Grid
    {
        Vector2I startingPoint;
        Vector2I cellsNumber;
        Hashtable<int, CellType> table;
    };

    bool running;
    CellType selectedType;
    Grid grid;

    int sourceKey;
    int targetKey;


    // searching 
    Heap<int> heap;
    
    // contains the cell's key as a key, 
    // and the distance to the source as the value
    Hashtable<int, int> distTo;

    // contains the cell's key as a key,
    // and the key of the cell before it
    Hashtable<int, int> from;

    bool pathFound;

    Vector2I currentPos;
    int currentKey;
    Vector2I currentEdge;

    // helper methods
    // ---------------------------------------------------------------------------------------------------------

    virtual bool isValid(int x, int y)
    {
        return x >= 0 && y >= 0 && x < grid.cellsNumber.x && y < grid.cellsNumber.y;
    }

    // used for converting screen position to grid position
    virtual Vector2I getGridCoordinates(Vector2 mouse)
    {
        Vector2I v;
        v.x = ((int)(mouse.x) - grid.startingPoint.x) / CELL_WIDTH;
        v.y = ((int)(mouse.y) - grid.startingPoint.y) / CELL_HEIGHT;
        return v;
    }

    // generates a key to be used by the grid
    virtual int generateKey(int x,  int y)
    {
        return x + y * grid.cellsNumber.x;
    }


    // generates the position relative to the grid from a key
    virtual Vector2I generatePos(int key)
    {
        int x = key % grid.cellsNumber.x;
        int y = key / grid.cellsNumber.x;
        return (Vector2I){.x = x, .y = y};
    }


    virtual void checkIteration()
    {
        if (currentEdge.x > MAX_X_EDGE)
        {
            currentEdge.x = MIN_X_EDGE;
            currentEdge.y += 1;
        }
        if (currentEdge.y > MAX_Y_EDGE)
        {
            if (heap.isEmpty())
            {
                std::cout<< "No Path is Found" << std::endl;
                std::abort();
            }
            currentEdge.x = MIN_X_EDGE;
            currentEdge.y = MIN_Y_EDGE;
            currentKey = heap.removeSmallest();
            currentPos = generatePos(currentKey);
        }
    }
    virtual void iterate()
    {
        currentEdge.x += 1;
    }

    virtual bool isCurrentEdgeValid()
    {
        // checking if the edge is valid in the grid (adding diagonally is not valid)
        return isValid(currentPos.x + currentEdge.x, currentPos.y + currentEdge.y) && abs(currentEdge.x) != abs(currentEdge.y);
    }


    virtual void addEdgeFrom(int edge, int fromEdge)
    {
        distTo.insert(edge, distTo.get(fromEdge) + 1);
        from.insert(edge, fromEdge);
        heap.add(edge, distTo.get(edge));
    }

    virtual bool putToGrid(int key, CellType ct)
    {

        if (running)
        {
            // cell is not inserted if it's place is occupied by
            // a wall or something of the same type
            if (grid.table.containsKey(key))
            {
                if (grid.table.get(key) >= ct) return false;
            } 
        }
        else if (ct == REMOVE)
        {
            // user can only remove the walls
            if (grid.table.containsKey(key) && (grid.table.get(key) == WALL))
            {
                grid.table.remove(key);
            }
            return false;
        }

        else if (grid.table.containsKey(key)) return false;

        else if (ct == SOURCE)
        {
            if (grid.table.containsKey(sourceKey)) grid.table.remove(sourceKey);

            sourceKey = key;
            grid.table.insert(sourceKey, SOURCE);
            return false;
        }
        else if (ct == TARGET)
        {
            if (grid.table.containsKey(targetKey)) grid.table.remove(targetKey);
            targetKey = key;
            grid.table.insert(targetKey, TARGET);
            return false;
        }

        if (sourceKey == key || targetKey == key) return false;

        grid.table.insert(key, ct);
        return true;
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

        pathFound = false;

        // make the source and the target in the middle
        // of the screen at opposite sides

        sourceKey = -1;
        targetKey = -1;

        putToGrid(grid.cellsNumber.y / 2 * grid.cellsNumber.x, TARGET);
        putToGrid(grid.cellsNumber.x - 1 + grid.cellsNumber.y / 2 * grid.cellsNumber.x, SOURCE);
    }




    virtual void select(CellType ct) {selectedType = ct;}
    virtual void run()
    {
        running = true;

        from.insert(sourceKey, -1);
        distTo.insert(sourceKey, 0);

        currentEdge.x = MIN_X_EDGE;
        currentEdge.y = MIN_Y_EDGE;

        currentKey = sourceKey;
        currentPos = generatePos(currentKey);
    }
    virtual void pause() {running = false;}



    virtual void press(Vector2 mouse)
    {
        if (running) {return;}
        Vector2I pos = getGridCoordinates(mouse);
        if (isValid(pos.x, pos.y))
        {
            putToGrid(generateKey(pos.x, pos.y), selectedType);
        }
    }


    virtual int getCurrentKey()
    {
        return currentKey;
    }

    virtual bool isRunning() {return running;}

    virtual bool isPathFound() {return pathFound;}

    // generates the rectangle to be drawn to the screen
    virtual Rectangle generateRect(int key)
    {
        int x = key % grid.cellsNumber.x;
        int y = key / grid.cellsNumber.x;
        Rectangle rect;
        rect.width = CELL_WIDTH;
        rect.height = CELL_HEIGHT;

        rect.x = x * CELL_WIDTH + grid.startingPoint.x;
        rect.y = y * CELL_HEIGHT + grid.startingPoint.y;

        return rect;
    }



    virtual void update(Hashtable<int, CellType>::HashIterator& iter)
    {
        if (running)
        {
            if (!pathFound)
            {
                
                checkIteration();
    
                if (isCurrentEdgeValid())
                {
                    int newKey = generateKey(currentPos.x + currentEdge.x, currentPos.y + currentEdge.y);

                    if (newKey == targetKey)
                    {
                        pathFound = true;
                        from.insert(targetKey, currentKey);
                    }
    
                    // only add the new cell if it's added to the grid successfully
                    if (putToGrid(newKey, CHECKED))
                    {
                        addEdgeFrom(newKey, currentKey);
                    }
                }
                iterate();
            }
            else
            {
                if (currentKey != sourceKey)
                {
                    putToGrid(currentKey, PATH);
                    currentKey = from.get(currentKey);
                }
            }
        }

        iter.begin(grid.table);
    }
};

#endif