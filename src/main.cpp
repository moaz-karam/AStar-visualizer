

#include <raylib.h>
#include <raygui.h>

#include "../data_structures/hashtable.h"

int main()
{

    Hashtable<int, int> ht;
    for (int i = 0; i < 100; i += 1)
    {
        ht.insert(i, i);
    }

    Hashtable<int, int>::HashIterator iter;

    iter.begin(ht);

    for (iter; iter.hasNext(); iter.next())
    {
        std::cout << iter.getData() << std::endl;
    }

    return 0;
}