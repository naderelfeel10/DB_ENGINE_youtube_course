#pragma once 


#include<string>
#include<vector>
#include<map>
#include"Column.h"
#include"RID.h"

#include"Buffer/BufferPoolManager.h"
#include"Indexing/BPLusTreeIndex.h"
#include"TableIterator.c++"
using namespace std;

class TableHeap{

private:
    //meta data about the table
    string table_name;
    int table_id;
    int num_of_tuples{0};
    //schema
    vector<Column> schema;

    //meta data about loading and storing into the HD 
    int first_page_id;
    int last_page_id;

    RID starting_rid;
    RID stopping_rid;

    //col_name : index
    map<string, BPLusTree*>B_plus_indexes;

public:
    TableHeap(BufferPoolManager* BPM, vector<Column> schema, string table_name, int table_id);
    BufferPoolManager* BPM;

    //crud 
    RID insert_tuple(Tuple tuple);
    Tuple* get_tuple(RID rid);
    RID update_tuple(RID rid, Tuple new_tuple);
    bool delete_tuple(RID rid);

    void addIndex(BPLusTree* index, string col_name);
    void fill_index(string col_name);

    bool check_schema(Tuple tuple);
    RID get_last_rid(){
        return this->stopping_rid;
    }

    int get_first_page_id(){
        return this->first_page_id;
    }
    int get_lst_page_id(){
        return this->last_page_id;
    }

    BPLusTree* get_index(string col_name);
};