#ifndef HEAP_H
#define HEAP_H

#include <iostream>
#include "./arraylist.hpp"

template <typename T>
class Heap
{
private:
    class HeapNode
    {
    private:
        T i;
        float p;
    public:
        HeapNode() {}
        HeapNode(T& item, float priority)
        {
            i = item;
            p = priority;
        }
        T& getI() {return i;}
        float getP() {return p;}
        void setI(T& item) {i = item;}
        void setP(float priority) {p = priority;}

    };


    ArrayList<HeapNode> items;



    int parentOf(int child)
    {
        return (child - 1) / 2;
    }

    int leftChild(int parent)
    {
        return 2 * parent + 1;
    }

    int rightChild(int parent)
    {
        return 2 * parent + 2;
    }


    void swap(int parent, int child)
    {
        HeapNode temp = items.get(child);
        items.set(child, items.get(parent));
        items.set(parent, temp);
    }


    bool smallerThan(int i1, int i2)
    {
        return items.get(i1).getP() < items.get(i2).getP();
    }
    int smallestOf(int parent)
    {
        int left = leftChild(parent);
        int right = rightChild(parent);

        if (smallerThan(right, left))
        {
            return right;
        }
        return left;
    }


    bool isValid(int index)
    {
        return index >= 0 && index < items.getSize();
    }

public:
    Heap() {}
    ~Heap() {}

    bool isEmpty() {return items.isEmpty();}

    void add(T& item, float p)
    {
        HeapNode n(item, p);


        int child = items.getSize();
        items.push(n);


        int parent = parentOf(child);

        while (smallerThan(child, parent))
        {
            swap(parent, child);
            child = parent;
            parent = parentOf(child);
        }

    }


    T removeSmallest()
    {
        if (isEmpty())
        {
            throw std::runtime_error("Can not remove the smallest from an empty Heap");
        }

        T temp = items.get(0).getI();
        items.set(0, items.pop());

        int parent = 0;

        int left = leftChild(parent);
        int right = rightChild(parent);
        int smallest;

        while (isValid(left))
        {
            // finding the smallest
            if (!isValid(right))
            {
                smallest = left;
            }
            else
            {
                if (smallerThan(right, left))
                {
                    smallest = right;
                }
                else
                {
                    smallest = left;
                }
            }

            // checking if the smallest childer is smaller than the parent
            if (smallerThan(smallest, parent))
            {
                swap(parent, smallest);
                parent = smallest;

                left = leftChild(parent);
                right = rightChild(parent);
            }
            else {break;}

        }

        return temp;
    }

    T& getSmallest()
    {
        return items.get(0).getI();
    }
    
    
    T& get(int i) {return items.get(i).getI();}
    float getP(int i) {return items.get(i).getP();}
    int getSize() {return items.getSize();}
    void clear() {items.clear();}
};

#endif