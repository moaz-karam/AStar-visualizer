#ifndef SEARCHER_H
#define SEARCHER_H

#include <math.h>
#include <time.h>

#include "../include/raylib.h"
#include "../data_structures/hashtable.hpp"
#include "../data_structures/heap.hpp"

#define MIN_CELL_DIMENSION 20.0f
#define ITERATIONS_PER_UPDATE 100
#define CELL_TIME_SECONDS 0.2f
#define CELLS_NUMBERS 400.0f

enum CellType
{
    CHECKED = 0, WALL = 1, PATH = 2, SOURCE = 3, TARGET = 4, REMOVE, 
};

const Color COLORS[] = {SKYBLUE, BROWN, YELLOW, ORANGE, DARKBLUE,};

struct Cell
{
    CellType ct;
    clock_t st;
};

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
        int cellDimension;
        Vector2 startingPoint;
        Vector2 cellsNumber;
        Vector2 dimensions;
        Hashtable<Vector2I, Cell> table;
    };

    bool running;
    CellType selectedType;
    Grid grid;

protected:
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

    float xDiff;
    float yDiff;
    
    bool dragging;



    // helper methods
    // ---------------------------------------------------------------------------------------------------------

    // used for converting screen position to grid position
    virtual Vector2I getGridCoordinates(Vector2 mouse)
    {
        Vector2I v;
        v.x = ((int)(mouse.x) - (grid.startingPoint.x + xDiff)) / grid.cellDimension;
        v.y = ((int)(mouse.y) - (grid.startingPoint.y + yDiff)) / grid.cellDimension;
        return v;
    }

    bool isMouseInGrid(Vector2 mouse)
    {
        return mouse.x > grid.startingPoint.x && mouse.y > grid.startingPoint.y
            && mouse.x < grid.startingPoint.x + grid.dimensions.x
            && mouse.y < grid.startingPoint.y + grid.dimensions.y;
    }

    virtual void addEdgeFrom(Vector2I vertex, Vector2I fromVertex)
    {
        distTo.insert(vertex, distTo.get(fromVertex) + 1);
        from.insert(vertex, fromVertex);
        heap.add(vertex, distTo.get(vertex));
    }

    virtual void applyDiffConstraints()
    {
        const float maxX = CELLS_NUMBERS * MIN_CELL_DIMENSION;
        const float maxY = CELLS_NUMBERS * MIN_CELL_DIMENSION;

        if (xDiff > 0) xDiff = 0;
        else if (-xDiff * MIN_CELL_DIMENSION / grid.cellDimension + grid.cellsNumber.x * MIN_CELL_DIMENSION > maxX)
        {
            xDiff = -(maxX - grid.cellsNumber.x * MIN_CELL_DIMENSION) * grid.cellDimension / MIN_CELL_DIMENSION;
        }
        if (yDiff > 0) yDiff = 0;
        else if (-yDiff * MIN_CELL_DIMENSION / grid.cellDimension + grid.cellsNumber.y * MIN_CELL_DIMENSION > maxY)
        {
            yDiff = -(maxY - grid.cellsNumber.y * MIN_CELL_DIMENSION) * grid.cellDimension / MIN_CELL_DIMENSION;
        }
    }

    virtual void applyRectConstraints(Rectangle* rect)
    {
        if (rect->x < grid.startingPoint.x)
        {
            rect->width = rect->width - (grid.startingPoint.x - rect->x);
            rect->x = grid.startingPoint.x;
        }
        else if (rect->x + rect->width > grid.startingPoint.x + grid.dimensions.x)
        {
            rect->width = grid.startingPoint.x + grid.dimensions.x - rect->x;
        }

        if (rect->y < grid.startingPoint.y)
        {
            rect->height = rect->height - (grid.startingPoint.y - rect->y);
            rect->y = grid.startingPoint.y;
        }
        else if (rect->y + rect->height > grid.startingPoint.y + grid.dimensions.y)
        {
            rect->height = (grid.startingPoint.y + grid.dimensions.y - rect->y);
        }
    }

    virtual bool isValidCell(Vector2I cell)
    {
        const float maxX = grid.dimensions.x / MIN_CELL_DIMENSION;
        const float maxY = grid.dimensions.y / MIN_CELL_DIMENSION;

        return cell.x >= 0 && cell.y >= 0 &&
                cell.x < CELLS_NUMBERS && cell.y < CELLS_NUMBERS;
    }

    virtual void resetSearch()
    {
        ArrayList<Vector2I> walls;
        Hashtable<Vector2I, Cell>::HashIterator iter;
        iter.begin(grid.table);
        while(iter.hasNext())
        {
            iter.next();
            if (iter.getValue().ct == WALL) walls.push(iter.getKey());
        }
        clear();
        for (int i = 0; i < walls.getSize(); i += 1) putToGrid(walls.get(i), WALL, true);
    }

    virtual bool putToGrid(Vector2I key, CellType ct, bool reset)
    {

        bool isAnimated = !reset && (ct == CHECKED || ct == PATH);

        if (!isValidCell(key)) return false;

        if (running)
        {
            // cell is not inserted if it's place is occupied by
            // a wall or something of the same type
            if (grid.table.containsKey(key))
            {
                if (grid.table.get(key).ct >= ct) return false;
            } 
        }
        else if (ct == REMOVE)
        {
            // user can only remove the walls
            if (grid.table.containsKey(key) && (grid.table.get(key).ct == WALL))
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
            grid.table.insert(sourcePos, {ct, clock() * isAnimated});
            return false;
        }
        else if (ct == TARGET)
        {
            if (grid.table.containsKey(targetPos)) grid.table.remove(targetPos);
            targetPos = key;
            grid.table.insert(targetPos, {ct, clock() * isAnimated});
            return false;
        }

        grid.table.insert(key, {ct, clock() * isAnimated});
        return true;
    }

    // ---------------------------------------------------------------------------------------------------------
    

public:
    explicit Searcher(Vector2 startingPos, Vector2 dimensions)
    {
        grid.startingPoint.x = startingPos.x;
        grid.startingPoint.y = startingPos.y;

        grid.cellDimension = MIN_CELL_DIMENSION;

        grid.dimensions = dimensions;

        grid.cellsNumber.x = dimensions.x / grid.cellDimension;
        grid.cellsNumber.y = dimensions.y / grid.cellDimension;
        

        running = false;

        selectedType = WALL;

        pathFound = false;

        const int diffCellsX = (CELLS_NUMBERS - grid.cellsNumber.x) / 2;
        const int diffCellsY = (CELLS_NUMBERS - grid.cellsNumber.y) / 2;

        targetPos = Vector2I{.x = diffCellsX, .y = diffCellsY + (int)grid.cellsNumber.y / 2};
        sourcePos = Vector2I{.x = diffCellsX + (int)grid.cellsNumber.x, .y = diffCellsY + (int)grid.cellsNumber.y / 2};

        putToGrid(sourcePos, SOURCE, false);
        putToGrid(targetPos, TARGET, false);

        // move everything left and up
        xDiff = -MIN_CELL_DIMENSION * diffCellsX;
        yDiff = -MIN_CELL_DIMENSION * diffCellsY;

        dragging = false;
    }

    explicit Searcher(Searcher* otherSearcher)
    {
        this->grid.startingPoint.x = otherSearcher->grid.startingPoint.x;
        this->grid.startingPoint.y = otherSearcher->grid.startingPoint.y;

        this->grid.cellDimension = otherSearcher->grid.cellDimension;

        this->grid.dimensions = otherSearcher->grid.dimensions;

        this->grid.cellsNumber.x = otherSearcher->grid.cellsNumber.x;
        this->grid.cellsNumber.y = otherSearcher->grid.cellsNumber.y;
        

        
        running = false;
        this->selectedType = otherSearcher->selectedType;
        this->pathFound = otherSearcher->pathFound;

        this->sourcePos = otherSearcher->sourcePos;
        this->targetPos = otherSearcher->targetPos;

        this->putToGrid(sourcePos, SOURCE, true);
        this->putToGrid(targetPos, TARGET, true);

        this->xDiff = otherSearcher->xDiff;
        this->yDiff = otherSearcher->yDiff;

        Hashtable<Vector2I, Cell>::HashIterator iter;
        iter.begin(otherSearcher->grid.table);

        while(iter.hasNext())
        {
            iter.next();
            if (iter.getValue().ct == WALL)
            {
                putToGrid(iter.getKey(), WALL, true);
            }
        }

        pathFound = false;
        dragging = false;
    }



    virtual void select(CellType ct)
    {
        selectedType = ct;
        // reset the search if some cell type is selected
        resetSearch();
    }
    virtual void run()
    {
        resetSearch();
        running = true;

        from.insert(sourcePos, Vector2I{.x = -1000, .y = -1000});
        distTo.insert(sourcePos, 0);

        heap.add(sourcePos, 0);

    }
    virtual void clear()
    {
        heap.clear();
        distTo.clear();
        from.clear();

        grid.table.clear();
        putToGrid(sourcePos, SOURCE, true);
        putToGrid(targetPos, TARGET, true);

        running = false;
        pathFound = false;
        dragging = false;
    }

    virtual void press(Vector2 mouse)
    {
        if (running || !isMouseInGrid(mouse)) {return;}
        Vector2I pos = getGridCoordinates(mouse);
        putToGrid(pos, selectedType, false);
    }

    virtual void drag(float x, float y)
    {
        xDiff += x;
        yDiff += y;
        applyDiffConstraints();
    }

    virtual void zoom(float n)
    {
        grid.cellDimension = std::max(n + grid.cellDimension, MIN_CELL_DIMENSION);

        grid.cellsNumber.x = grid.dimensions.x / grid.cellDimension;
        grid.cellsNumber.y = grid.dimensions.y / grid.cellDimension;
        applyDiffConstraints();
    }

    virtual Vector2I getCurrentPos()
    {
        return currentPos;
    }

    virtual bool isRunning() {return running;}

    virtual bool isPathFound() {return pathFound;}

    // returns true if a position is to be drawn to the screen
    virtual bool isValidRect(Vector2I pos)
    {

        float xCellDiff = (float)xDiff / (float)grid.cellDimension;
        float yCellDiff = (float)yDiff / (float)grid.cellDimension;

        return xCellDiff + pos.x > -1 && yCellDiff + pos.y > -1 &&
                xCellDiff + pos.x < grid.cellsNumber.x && yCellDiff + pos.y < grid.cellsNumber.y;
    }

    // generates the rectangle to be drawn to the screen
    virtual void generateRect(Vector2I pos, Rectangle* rect, Cell* cell)
    {
        clock_t now = clock();

        float timeDiff = (float)(now - cell->st) / CLOCKS_PER_SEC;

        float dimension = (timeDiff / CELL_TIME_SECONDS) * grid.cellDimension;

        rect->width = dimension < grid.cellDimension ? dimension : grid.cellDimension;
        rect->height = rect->width;

        float cellDiff = (grid.cellDimension - rect->width) / 2.0f;
        
        rect->x = pos.x * grid.cellDimension + grid.startingPoint.x + xDiff + cellDiff;
        rect->y = pos.y * grid.cellDimension + grid.startingPoint.y + yDiff + cellDiff;


        // conditions are added to prevent drawing outside the grid
        applyRectConstraints(rect);
    }

    virtual bool isColumn(int colNumber)
    {

        int firstX = (int)xDiff % grid.cellDimension + grid.startingPoint.x;

        int x = firstX + (colNumber - (firstX >= grid.startingPoint.x)) * grid.cellDimension;

        return x < grid.startingPoint.x + grid.dimensions.x;
    }

    virtual void getColumn(int colNumber, Vector2* sp, Vector2* ep)
    {
        sp->y = grid.startingPoint.y;
        ep->y = grid.startingPoint.y + grid.dimensions.y;

        int firstX = (int)xDiff % grid.cellDimension + grid.startingPoint.x;

        int x = firstX + (colNumber - (firstX >= grid.startingPoint.x)) * grid.cellDimension;

        sp->x = x;
        ep->x = x;
    }

    virtual bool isRow(int rowNumber)
    {
        int firstY = (int)yDiff % grid.cellDimension + grid.startingPoint.y;
        
        int y = firstY + (rowNumber - (firstY >= grid.startingPoint.y)) * grid.cellDimension;

        return y <= grid.startingPoint.y + grid.dimensions.y;
    }

    virtual void getRow(int rowNumber, Vector2* sp, Vector2* ep)
    {

        sp->x = grid.startingPoint.x;
        ep->x = grid.startingPoint.x + grid.dimensions.x;

        int firstY = (int)yDiff % grid.cellDimension + grid.startingPoint.y;

        int y = firstY + (rowNumber - (firstY >= grid.startingPoint.y)) * grid.cellDimension;

        sp->y = y;
        ep->y = y;
    }

    virtual void update(Hashtable<Vector2I, Cell>::HashIterator& iter)
    {
        for (int i = 0; i < ITERATIONS_PER_UPDATE; i += 1)
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
                
                            // diagonal is not allowed
                            if (abs(x) != abs(y))
                            {
                                Vector2I newPos = (Vector2I){currentPos.x + x, currentPos.y + y};
            
                                if (newPos == targetPos)
                                {
                                    pathFound = true;
                                    from.insert(targetPos, currentPos);
                                }
                
                                // only add the new cell if it's added to the grid successfully
                                if (putToGrid(newPos, CHECKED, false))
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
                        putToGrid(currentPos, PATH, false);
                        currentPos = from.get(currentPos);
                    }
                }
            }
        }

        iter.begin(grid.table);
    }
};





class Dijkstra : public Searcher
{
public:
    Dijkstra(Vector2 startingPos, Vector2 dimensions):Searcher(startingPos, dimensions)
    {
    }
    Dijkstra(Searcher* otherSearcher):Searcher(otherSearcher)
    {
    }
};






class AStar : public Searcher
{

private:
    void addEdgeFrom(Vector2I vertex, Vector2I fromVertex) override
    {
        distTo.insert(vertex, distTo.get(fromVertex) + 1);
        from.insert(vertex, fromVertex);

        float dx = vertex.x - targetPos.x;
        float dy = vertex.y - targetPos.y;
        float d = sqrt(pow(dx, 2) + pow(dy, 2));

        heap.add(vertex, distTo.get(vertex) + d);
    }
public:
    AStar(Vector2 startingPos, Vector2 dimensions):Searcher(startingPos, dimensions)
    {
    }
    AStar(Searcher* otherSearcher):Searcher(otherSearcher)
    {
    }
};






#endif