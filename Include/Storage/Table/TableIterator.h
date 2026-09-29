#pragma once 

#include"TableHeap.h"

class TableIterator{

private :
    RID curr_rid;
    RID stopping_rid;
    TableHeap* table_heap;

public:
    TableIterator(TableHeap* table_heap, RID starting_rid, RID stopping_rid):
        table_heap(table_heap), curr_rid(starting_rid), stopping_rid(stopping_rid){};

    TableIterator& operator++();
    Tuple operator*();
    bool end();
};