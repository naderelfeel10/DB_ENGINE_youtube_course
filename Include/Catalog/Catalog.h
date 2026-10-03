#pragma once 

#include<unordered_map>
#include<string>
#include"Storage/Table/TableHeap.h"


using namespace std;

// a representation of the table info in the catalog
class TableInfo{

private:

    //table meta data
    string table_name;
    int table_id;
    vector<Column>schema;
    int first_page_id;
    TableHeap* table_heap;

public:

    TableInfo():table_name("EMPTY"),table_id(-1),schema({}),first_page_id(-1), table_heap(nullptr){};

    TableInfo(TableHeap*table_heap ,string table_name, int table_id, vector<Column>schema, int first_page_id)
    :table_heap(table_heap), table_name(table_name), table_id(table_id), schema(schema), first_page_id(first_page_id){};

    int serializeTableInfo(char* buffer);
    int deserialize_tableInfo(BufferPoolManager* DB_PPM, const char* buffer);    
    
    //getters 
    TableHeap* get_table_heap(){
        return this->table_heap;
    }
    string get_table_name(){
        return this->table_name;
    }
    int get_table_id(){
        return this->table_id;
    }
    vector<Column> get_table_schema(){
        return this->schema;
    }

};


class Catalog{

private:
    
    //save tables in my system 
    //a map of table_id : table_info
    unordered_map<int, TableInfo*> tables_ids_map;
    unordered_map<string, TableInfo*> tables_names_map;

    BufferPoolManager* catalog_BPM;
    BufferPoolManager* DB_BPM;

    //track last table_id : 
    int lst_table_id{0};

    //track pages_ids of the catalog
    int first_page_id{-1};
    int lst_page_id{-1};

public:
    
    Catalog(BufferPoolManager* catalog_BPM ,BufferPoolManager* DB_BPM ,bool create_new);    

    TableInfo* create_table(string table_name, vector<Column>schema);
   
    void save_catalog();
    void load_catalog();

    //getters 
    int get_number_of_tables(){
        return tables_ids_map.size();
    }

    unordered_map<int, TableInfo*> get_tables_ids_map(){
        return this->tables_ids_map;
    }

    unordered_map<string, TableInfo*> get_tables_names_map(){
        return this->tables_names_map;
    }

    TableInfo* get_table(string table_name){
        return tables_names_map[table_name];
    }

    TableInfo* get_table(int table_id){
        return tables_ids_map[table_id];
    }

};