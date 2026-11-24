#pragma once 


#include<string>
#include<vector>
#include<map>
#include"Column.h"
#include"RID.h"

#include"Buffer/BufferPoolManager.h"
#include"Storage/Indexing/BPLusTreeIndex.h"
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
    map<string, BPLusTree*>pplusindexes;

public:
    TableHeap(BufferPoolManager* BPM, vector<Column> schema, string table_name, int table_id);
    // reopen a table using its existing first page
    TableHeap(BufferPoolManager* BPM, vector<Column> schema, string table_name, int table_id, int first_page_id);
    BufferPoolManager* BPM;

    //crud 
    RID insert_tuple(Tuple tuple);
    Tuple* get_tuple(RID rid);
    RID update_tuple(RID rid, Tuple new_tuple);
    bool delete_tuple(RID rid);

    void addIndex(BPLusTree* index, string col_name);

    // attach an existing index without filling it again
    void attachIndex(BPLusTree* index, string col_name);

    BPLusTree* getIndex(string col_name);
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

};
