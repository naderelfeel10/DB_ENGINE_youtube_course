#pragma once 


#include"Storage/Table/TableHeap.h"
#include<string>
using namespace std;

class TableInfo{

private : 
    string table_name;
    int table_id;
    vector<Column>schema;
    int first_page_id;

public: 

    TableInfo():table_name("EMPTPY"), table_id(-1), schema({}), first_page_id(-1){};
    TableInfo(string table_name, int table_id, vector<Column>schema, int first_page_id):table_name(table_name), table_id(table_id),
                                    schema(schema), first_page_id(first_page_id){};

                        
    int load_table_info(BufferPoolManager*DB_BPM,char* page_buffer);
    int save_table_info(char* buffer);

    int get_table_id(){
        return this->table_id;
    }

    int get_table_frist_page_id(){
        return this->first_page_id;
    }

    string get_table_name(){
        return this->table_name;
    }

    vector<Column> get_table_schema(){
        return this->schema;
    }

};


// this catalog file holds the meta-data of one DB file 
// here i need to save all of this DB tables
class Catalog{

private:
    //save the tables on the db file 

    //map table_id, table_info
    unordered_map<int, TableInfo*> tables_ids_map;
    //map table_name, table_info
    unordered_map<string, TableInfo*> tables_names_map;

    int first_pag_id{-1};

    int lst_table_id{0};
    BufferPoolManager* catalog_BPM;
    BufferPoolManager* DB_BPM;
    
public:

    Catalog(BufferPoolManager* catalog_BPM, BufferPoolManager* DB_BPM, bool is_new);

    TableInfo* create_table(string table_name, vector<Column>schema);
    TableInfo* get_table(string table_name);
    TableInfo* get_table(int table_id);

    void save_catalog();
    void load_catalog();

};