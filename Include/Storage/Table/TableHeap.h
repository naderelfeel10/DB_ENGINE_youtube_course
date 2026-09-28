#pragma once


#include<iostream>
#include<string>
#include<vector>
#include"Column.h"
#include"RID.h"
#include"Buffer/BufferPoolManager.h"

using namespace std;

//represent an actual database table 

class TableHeap{
    
private:

    //table meta-data
    string table_name;
    int table_id;
    int num_of_tuples{0};
    //schema : 
    vector<Column> schema;  
    
    //track table page_id:
    int first_page_id{-1};
    int last_page_id{-1};
    
    RID starting_rid;
    RID stoping_rid;

public:

    TableHeap(BufferPoolManager* BPM, vector<Column>schema, string table_name, int table_id);
    
    BufferPoolManager* BPM;

    //crud:
    RID insert_tuple(Tuple tuple);
    Tuple* get_tuple(RID rid);
    RID update_tuple(RID rid, Tuple tuple);
    bool delete_tuple(RID rid);

    bool check_schema(Tuple tuple);

    RID get_starting_rid();
    RID get_stopping_rid();

    int get_table_id();
    string get_table_name();

    void set_table_name(string new_name);
    
    void delete_table();
};