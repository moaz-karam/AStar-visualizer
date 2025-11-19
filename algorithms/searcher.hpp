#ifndef SEARCHER_H
#define SEARCHER_H

#include <raylib.h>
#include "../data_structures/hashtable.hpp"
#include "../data_structures/heap.hpp"

#define CELL_WIDTH 50
#define CELL_HEIGHT 50

enum CellType
{
    CHECKED = 0, WALL = 1, PATH = 2, SOURCE = 3, TARGET = 4, REMOVE, 
};

const Color COLORS[] = {SKYBLUE, BROWN, YELLOW, DARKGREEN, DARKBLUE,};



struct Vector2I
{
    int x;
    int y;
    Vector2I& operator=(const Vector2I& v1)
    {
        this->x = v1.x;
        this->y = v1.y;
        return *this;
    }

    inline bool operator==(const Vector2I& rhs)
    {
        return this->x == rhs.x && this->y == rhs.y;
    }

    inline bool operator!=(const Vector2I& rhs)
    {
        return this->x != rhs.x || this->y != rhs.y;
    }
    size_t operator()(const Vector2I &p) const
    {
        return std::hash<int>()(this->x + this->y);
    }

};

template<>
struct std::hash<Vector2I>
{
    size_t operator()(const Vector2I &p) const
    {
        return std::hash<int>()(p.x + p.y);
    }
};

class Searcher
{
private:




    struct Grid
    {
        Vector2I startingPoint;
        Vector2I cellsNumber;
        Hashtable<Vector2I, CellType> table;
    };

    bool running;
    CellType selectedType;
    Grid grid;

    Vector2I sourcePos;
    Vector2I targetPos;


    // searching 
    Heap<Vector2I> heap;
    
    // contains the cell's key as a key, 
    // and the distance to the source as the value
    Hashtable<Vector2I, int> distTo;

    // contains the cell's key as a key,
    // and the key of the cell before it
    Hashtable<Vector2I, Vector2I> from;

    bool pathFound;

    Vector2I currentPos;

    int xDiff;
    int yDiff;
    
    bool dragging;

    // helper methods
    // ---------------------------------------------------------------------------------------------------------

    // used for converting screen position to grid position
    virtual Vector2I getGridCoordinates(Vector2 mouse)
    {
        Vector2I v;
        v.x = ((int)(mouse.x) - (grid.startingPoint.x + xDiff)) / CELL_WIDTH;
        v.y = ((int)(mouse.y) - (grid.startingPoint.y + yDiff)) / CELL_HEIGHT;
        return v;
    }

    virtual void addEdgeFrom(Vector2I vertex, Vector2I fromVertex)
    {
        distTo.insert(vertex, distTo.get(fromVertex) + 1);
        from.insert(vertex, fromVertex);
        heap.add(vertex, distTo.get(vertex));
    }

    virtual bool putToGrid(Vector2I key, CellType ct)
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
            if (grid.table.containsKey(sourcePos)) grid.table.remove(sourcePos);

            sourcePos = key;
            grid.table.insert(sourcePos, SOURCE);
            return false;
        }
        else if (ct == TARGET)
        {
            if (grid.table.containsKey(targetPos)) grid.table.remove(targetPos);
            targetPos = key;
            grid.table.insert(targetPos, TARGET);
            return false;
        }

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

        sourcePos = Vector2I{.x = -1000,.y = -1000};
        targetPos = Vector2I{.x = -1000,.y = -1000};

        putToGrid((Vector2I){.x = 0, .y = grid.cellsNumber.y / 2}, TARGET);
        putToGrid((Vector2I){.x = grid.cellsNumber.x - 1, .y = grid.cellsNumber.y / 2}, SOURCE);

        xDiff = 0;
        yDiff = 0;

        dragging = false;
    }




    virtual void select(CellType ct) {selectedType = ct;}
    virtual void run()
    {
        running = true;

        from.insert(sourcePos, Vector2I{.x = -1000, .y = -1000});
        distTo.insert(sourcePos, 0);

        heap.add(sourcePos, 0);

    }
    virtual void pause() {running = false;}



    virtual void press(Vector2 mouse)
    {
        if (running) {return;}
        Vector2I pos = getGridCoordinates(mouse);
        putToGrid(pos, selectedType);
    }

    virtual void startDragging(){dragging = true;}
    virtual void stopDragging() {dragging = false;}

    virtual bool isDragging() {return dragging;}

    virtual void drag(int x, int y)
    {
        xDiff += x;
        yDiff += y;
    }

    virtual Vector2I getCurrentPos()
    {
        return currentPos;
    }

    virtual bool isRunning() {return running;}

    virtual bool isPathFound() {return pathFound;}

    // generates the rectangle to be drawn to the screen
    virtual void generateRect(Vector2I pos, Rectangle* rect)
    {
        rect->width = CELL_WIDTH;
        rect->height = CELL_HEIGHT;

        rect->x = pos.x * CELL_WIDTH + grid.startingPoint.x + xDiff;
        rect->y = pos.y * CELL_HEIGHT + grid.startingPoint.y + yDiff;

    }

    virtual bool isInRange(Rectangle rect)
    {
        return rect.x >= -CELL_WIDTH && rect.x <= (grid.cellsNumber.x + 1) * CELL_WIDTH
            && rect.y >= -CELL_HEIGHT && rect.y <= (grid.cellsNumber.y + 1) * CELL_HEIGHT;
    }


    virtual void update(Hashtable<Vector2I, CellType>::HashIterator& iter)
    {
        if (running)
        {
            if (!pathFound)
            {
                
                if (!heap.isEmpty())
                {
                    currentPos = heap.removeSmallest();
                }

                for (int y = -1; y <= 1; y += 1)
                {
                    for (int x = -1; x <= 1; x += 1)
                    {
            
                        if (abs(x) != abs(y))
                        {
                            Vector2I newPos = (Vector2I){currentPos.x + x, currentPos.y + y};
        
                            if (newPos == targetPos)
                            {
                                pathFound = true;
                                from.insert(targetPos, currentPos);
                            }
            
                            // only add the new cell if it's added to the grid successfully
                            if (putToGrid(newPos, CHECKED))
                            {
                                addEdgeFrom(newPos, currentPos);
                            }
                        }

                    }
                }
            }
            else
            {
                if (currentPos != sourcePos)
                {
                    putToGrid(currentPos, PATH);
                    currentPos = from.get(currentPos);
                }
            }
        }

        iter.begin(grid.table);
    }
};

#endif