#ifndef SEARCHER_H
#define SEARCHER_H

#include <ctime>
#include <math.h>
#include <time.h>

#include "../include/raylib/src/raylib.h"
#include "../data_structures/hashtable.hpp"
#include "../data_structures/heap.hpp"

#define MIN_CELL_DIMENSION 20.0f
#define ITERATIONS_PER_UPDATE 20
#define SIZE_ANIMATION_TIME 0.2f
#define LINEAR_ANIMATION_TIME 0.5f
#define CELLS_NUMBERS 400.0f


#define SOURCE_COLOR GetColor((int)0xFF6F00FF)
#define TARGET_COLOR DARKBLUE
#define WALL_COLOR BROWN

enum CellType
{
    CHECKED = 0, WALL = 1, PATH = 2, SOURCE = 3, TARGET = 4, REMOVE, 
};

const Color COLORS[] = {SKYBLUE, WALL_COLOR, YELLOW, SOURCE_COLOR, TARGET_COLOR,};

struct Cell
{
    CellType ct;
    double st;
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

    Vector2I& operator=(const Vector2& v1)
    {
        this->x = (int)v1.x;
        this->y = (int)v1.y;
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

    // used for animating the source and the target
    // making them move across the screen if their place is changed
    struct LinearAnimation
    {
        Vector2 lastPos;
        float distance;
        float slope;
    };

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
    Vector2I lastInsertedCell;
    LinearAnimation sourceAnimation;
    LinearAnimation targetAnimation;

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

    bool isMouseInGrid(Vector2 newMouse)
    {
        return newMouse.x > grid.startingPoint.x && newMouse.y > grid.startingPoint.y
            && newMouse.x < grid.startingPoint.x + grid.dimensions.x
            && newMouse.y < grid.startingPoint.y + grid.dimensions.y;
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
        return cell.x >= 0 && cell.y >= 0 &&
                cell.x < CELLS_NUMBERS && cell.y < CELLS_NUMBERS;
    }

    virtual void resetSearch()
    {
        ArrayList<Vector2I> walls;
        Hashtable<Vector2I, Cell>::HashIterator iter;
        iter.begin(grid.table);
        while (iter.hasNext())
        {
            iter.next();
            if (iter.getValue().ct == WALL) walls.push(iter.getKey());
        }
        clear();
        for (int i = 0; i < walls.getSize(); i += 1) putToGrid(walls.get(i), WALL, 0);
    }

    virtual Vector2 getAnimationPos(CellType ct, double now, double st)
    {
        Vector2 pos;
        double timeDiff = now - st;
        if (ct == SOURCE)
        {
            if (timeDiff > LINEAR_ANIMATION_TIME)
            {
                pos = Vector2{.x = (float)sourcePos.x, .y = (float)sourcePos.y};
            }
            else
            {
                pos = sourceAnimation.lastPos;
                pos.x += sourceAnimation.distance * cos(sourceAnimation.slope) * (timeDiff / LINEAR_ANIMATION_TIME);
                pos.y += sourceAnimation.distance * sin(sourceAnimation.slope) * (timeDiff / LINEAR_ANIMATION_TIME);
            }
        }
        else if (ct == TARGET)
        {
            if (timeDiff > LINEAR_ANIMATION_TIME)
            {
                pos = Vector2{.x = (float)targetPos.x, .y = (float)targetPos.y};
            }
            else
            {
                pos = targetAnimation.lastPos;
                pos.x += targetAnimation.distance * cos(targetAnimation.slope) * (timeDiff / LINEAR_ANIMATION_TIME);
                pos.y += targetAnimation.distance * sin(targetAnimation.slope) * (timeDiff / LINEAR_ANIMATION_TIME);
            }
        }
        return pos;
    }

    virtual void handleAnimation(Vector2I pos, Rectangle* rect, Cell* cell)
    {
        double now = GetTime();

        double timeDiff = now - cell->st;

        Vector2 cellPos = {.x = (float)pos.x, .y = (float)pos.y};
        if (cell->ct == SOURCE)
        {
            cellPos = getAnimationPos(SOURCE, now, cell->st);
        }
        else if (cell->ct == TARGET)
        {
            cellPos = getAnimationPos(TARGET, now, cell->st);
        }

        float dimension = cell->ct != TARGET && cell->ct != SOURCE ? (timeDiff / SIZE_ANIMATION_TIME) * grid.cellDimension : grid.cellDimension;
        rect->width = dimension < grid.cellDimension ? dimension : grid.cellDimension;
        rect->height = rect->width;

        float center = (grid.cellDimension - rect->width) / 2.0f;
        
        rect->x = cellPos.x * grid.cellDimension + grid.startingPoint.x + xDiff + center;
        rect->y = cellPos.y * grid.cellDimension + grid.startingPoint.y + yDiff + center;
    }

    virtual bool putToGrid(Vector2I key, CellType ct, double time)
    {
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
            if (grid.table.containsKey(sourcePos))
            {
                sourceAnimation.lastPos = getAnimationPos(SOURCE, GetTime(), grid.table.get(sourcePos).st);

                int sourceYDiff = key.y - sourceAnimation.lastPos.y;
                int sourceXDiff = key.x - sourceAnimation.lastPos.x;

                sourceAnimation.distance = sqrt(pow(sourceYDiff, 2) + pow(sourceXDiff, 2));
                sourceAnimation.slope = atan2(sourceYDiff, sourceXDiff);      
                grid.table.remove(sourcePos);
            }

            grid.table.insert(key, {ct, time});
            sourcePos = key;
            return false;
        }
        else if (ct == TARGET)
        {
            if (grid.table.containsKey(targetPos))
            {
                targetAnimation.lastPos = getAnimationPos(TARGET, GetTime(), grid.table.get(targetPos).st);

                int targetYDiff = key.y - targetAnimation.lastPos.y;
                int targetXDiff = key.x - targetAnimation.lastPos.x;
                
                targetAnimation.distance = sqrt(pow(targetYDiff, 2) + pow(targetXDiff, 2));
                targetAnimation.slope = atan2(targetYDiff, targetXDiff);
                grid.table.remove(targetPos);
            }
            grid.table.insert(key, {ct, time});
            

            targetPos = key;
            return false;
        }

        grid.table.insert(key, {ct, time});
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

        targetPos = Vector2I{.x = diffCellsX + (int)grid.cellsNumber.x / 5, .y = diffCellsY + (int)grid.cellsNumber.y / 2};
        sourcePos = Vector2I{.x = diffCellsX + 4 * (int)grid.cellsNumber.x / 5, .y = diffCellsY + (int)grid.cellsNumber.y / 2};

        targetAnimation.lastPos = (Vector2){.x = (float)targetPos.x, .y = (float)targetPos.y};
        sourceAnimation.lastPos = (Vector2){.x = (float)sourcePos.x, .y = (float)sourcePos.y};

        putToGrid(sourcePos, SOURCE, 0);
        putToGrid(targetPos, TARGET, 0);

        // move everything left and up
        xDiff = -MIN_CELL_DIMENSION * diffCellsX;
        yDiff = -MIN_CELL_DIMENSION * diffCellsY;

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

        this->putToGrid(sourcePos, SOURCE, otherSearcher->grid.table.get(sourcePos).st);
        this->putToGrid(targetPos, TARGET, otherSearcher->grid.table.get(targetPos).st);

        this->xDiff = otherSearcher->xDiff;
        this->yDiff = otherSearcher->yDiff;

        Hashtable<Vector2I, Cell>::HashIterator iter;
        iter.begin(otherSearcher->grid.table);

        while (iter.hasNext())
        {
            iter.next();
            if (iter.getValue().ct == WALL)
            {
                putToGrid(iter.getKey(), WALL, 0);
            }
        }

        pathFound = false;
        this->sourceAnimation = otherSearcher->sourceAnimation;
        this->targetAnimation = otherSearcher->targetAnimation;
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

        double sourceTime = grid.table.get(sourcePos).st;
        double targetTime = grid.table.get(targetPos).st;

        grid.table.clear();
        putToGrid(sourcePos, SOURCE, sourceTime);
        putToGrid(targetPos, TARGET, targetTime);

        running = false;
        pathFound = false;
    }

    virtual void press(Vector2 newMouse, bool isLeftPressed)
    {
        if (!isLeftPressed || running || !isMouseInGrid(newMouse)) {
            this->lastInsertedCell = Vector2I{.x = -1, .y = -1};
            return;
        }
        Vector2I newCell = getGridCoordinates(newMouse);
        lastInsertedCell = lastInsertedCell.x > -1 && lastInsertedCell.y > -1 ? lastInsertedCell : newCell;

        int xCellDiff = newCell.x - lastInsertedCell.x;
        int yCellDiff = newCell.y - lastInsertedCell.y;
        float slope = atan2(yCellDiff, xCellDiff);
        float diffLength = sqrt(pow(xCellDiff, 2) + pow(yCellDiff, 2));

        for (int i = 0; i <= diffLength; i += 1)
        {
            Vector2I tempCell = Vector2I{.x = (int)(lastInsertedCell.x + i * cos(slope)), .y = (int)(lastInsertedCell.y + i * sin(slope))};
            putToGrid(tempCell, selectedType, selectedType == WALL ? 0 : GetTime());
        }
        lastInsertedCell = newCell;
    }

    virtual void drag(float x, float y)
    {
        xDiff += x;
        yDiff += y;
        applyDiffConstraints();
    }

    virtual void zoom(Vector2 mouse, float n)
    {
        if (abs(n) == 0 || !isMouseInGrid(mouse)) return;

        Vector2I mouseCell = getGridCoordinates(mouse);
        
        float oldCellDimension = grid.cellDimension;
        grid.cellDimension = std::max(2 * n + grid.cellDimension, MIN_CELL_DIMENSION);

        float sizeChange = grid.cellDimension - oldCellDimension;

        float xDirect = -sizeChange * mouseCell.x;
        float yDirect = -sizeChange * mouseCell.y;

        grid.cellsNumber.x = grid.dimensions.x / grid.cellDimension;
        grid.cellsNumber.y = grid.dimensions.y / grid.cellDimension;

        drag(xDirect, yDirect);

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
    
        handleAnimation(pos, rect, cell);
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
                                if (putToGrid(newPos, CHECKED, GetTime()))
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
                        putToGrid(currentPos, PATH, GetTime());
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

        heap.add(vertex, distTo.get(vertex) * 0.75 + d);
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