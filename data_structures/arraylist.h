#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

template <typename T>
class ArrayList
{
private:
    T* items;
    int size;
    int length;
   
    void resizeUp()
    {
        length *= 2;
        T* temp = items;

        items = new T[length];

        for (int i = 0; i < size; i += 1)
        {
            items[i] = temp[i];
        }

        delete [] temp;
    }


    void resizeDown()
    {
        length *= 0.5;
        T* temp = items;
        items = new T[length];
        for (int i = 0; i < size; i += 1)
        {
            items[i] = temp[i];
        }
        delete [] temp;
    }
public:
    ArrayList()
    {
        size = 0;
        length = 4;
        items = new T[length];
    }
    ~ArrayList()
    {
        delete [] items;
    }

    bool isEmpty() {return size == 0;}

    void push(T item)
    {
        if (size / length == 1)
        {
            resizeUp();
        }
        items[size] = item;
        size += 1;
    }


    T& pop()
    {
        if (isEmpty())
        {
            std::runtime_error("Can not pop from an empty list");
        }
        if (length > 32 && (float)size / (float)length < 0.25)
        {
            resizeDown();
        }
        size -= 1;
        return items[size];
    }


    void set(int i, T item) {items[i] = item;}
    T& get(int i) const {return items[i];}
    int getSize() const {return size;}

};

#endif