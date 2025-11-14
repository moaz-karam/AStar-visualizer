#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <iostream>

#include "./arraylist.hpp"

template <typename K, typename V>
class Hashtable
{
private:

    class HashNode
    {
    private:
        K key;
        V value;

    public:
        HashNode() {};
        HashNode(K& k, V& v) {key = k; value = v;}
        K& getKey() {return key;}
        V& getValue() {return value;}

        void setKey(K& k) {key = k;}
        void setValue(V& v) {value = v;}
    };

    ArrayList<HashNode>* arrays;
    int size;
    int length;

    std::hash<K> h;


    int hash(K key)
    {
        return h(key) % length;
    }

    void resize(float factor)
    {
        length *= factor;
        ArrayList<HashNode>* temp = arrays;
        arrays = new ArrayList<HashNode>[length];

        size = 0;

        for (int i = 0; i < length / factor; i += 1)
        {
            for (int j = 0; j < temp[i].getSize(); j += 1)
            {
                HashNode node = temp[i].get(j);
                insert(node.getKey(), node.getValue());
            }
        }
        delete [] temp;
    }

public:
    Hashtable()
    {
        size = 0;
        length = 4;

        arrays = new ArrayList<HashNode>[length];
    }
    ~Hashtable()
    {
        delete [] arrays;
    }

    void insert(K k, V& v)
    {
        if ((float)size / (float)length >= 1)
        {
            resize(2);
        }
        
        HashNode n(k, v);
        int index = hash(k);

        for (int i = 0; i < arrays[index].getSize(); i += 1)
        {
            if (k == arrays[index].get(i).getKey())
            {
                arrays[index].get(i).setValue(v);
                return;
            }
        }
        arrays[index].push(n);
        
        size += 1;
    }

    bool containsKey(K& k)
    {
        int index = hash(k);

        for (int i = 0; i < arrays[index].getSize(); i += 1)
        {
            if (arrays[index].get(i).getKey() == k)
            {
                return true;
            }
        }
        return false;
    }


    V& get(K k)
    {
        int index = hash(k);

        for (int i = 0; i < arrays[index].getSize(); i += 1)
        {
            if (arrays[index].get(i).getKey() == k)
            {
                return arrays[index].get(i).getValue();
            }
        }
        throw std::runtime_error("The item with the key value does not exist in the table");
    }

    void set(K& k, V& v)
    {
        int index = hash(k);

        for (int i = 0; i < arrays[index].getSize(); i += 1)
        {
            if (arrays[index].get(i).getKey() == k)
            {
                arrays[index].get(i).setValue(v);
                return;
            }
        }
        throw std::runtime_error("The item with the key value does not exist in the table");
    }

    V& remove(K& k)
    {
        if (length > 32 && ((float)size / (float)length) < 0.25)
        {
            resize(0.5);
        }

        int index = hash(k);
        for (int i = 0; i < arrays[index].getSize(); i += 1)
        {
            if (arrays[index].get(i).getKey() == k)
            {
                // set the deleted element to the last element
                V temp = arrays[index].get(i).getValue();
                arrays[index].set(i, arrays[index].pop());
                
                size -= 1;

                return temp;
            }
        }

        throw std::runtime_error("The item with the key value does not exist in the table");
    }

    void printSizes()
    {
        for (int i = 0; i < length; i += 1)
        {
            std::cout << arrays[i].getSize() << std::endl;
        }
        std::cout << length << std::endl;
    }

    int getSize() {return size;}

    class HashIterator
    {
    private:

        Hashtable<K, V>* ht;

        int index1;
        int index2;
        
        int counter;
    public:
        void begin(Hashtable<K, V>& htable)
        {
            ht = &htable;
            index1 = 0;
            index2 = -1;
            counter = 0;

            next();
        }

        void next()
        {
            if (counter > ht->size)
            {
                throw std::runtime_error("The iterator has not next value");
            }

            while (index2 + 1 >= ht->arrays[index1].getSize())
            {
                index2 = -1;
                index1 += 1;
            }
            index2 += 1;
            counter += 1;
        }

        bool hasNext()
        {
            return counter <= ht->size;
        }

        V& getData()
        {
            return ht->arrays[index1].get(index2).getValue();
        }

    };

};

#endif