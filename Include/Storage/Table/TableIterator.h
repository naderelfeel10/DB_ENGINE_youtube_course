#pragma once 

#include"Storage/Table/RID.h"
#include"Storage/Page/Tuple.h"
class TableHeap;

class TableIterator{

private :
    RID curr_rid;
    RID stopping_rid;
    TableHeap* table_heap;

public:

    TableIterator(TableHeap* table_heap, RID starting_rid, RID stopping_rid);
    void skip_deleted();

    TableIterator& operator++();
    Tuple operator*();
    bool end();

    RID get_curr_rid(){return this->curr_rid;}
};
