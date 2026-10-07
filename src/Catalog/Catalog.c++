#include"Catalog/Catalog.h"


Catalog::Catalog(BufferPoolManager* catalog_BPM, BufferPoolManager* DB_BPM, bool is_new){

    this->catalog_BPM = catalog_BPM;
    this->DB_BPM = DB_BPM;

    //check if it's new or not
    if(is_new == true){
        //create a new catalog file on the HD
        first_pag_id = catalog_BPM->newPage();
        cout<<"frist page id : "<< first_pag_id<<endl;
        //save it
        save_catalog();
    }else{
        // load the existing catalog file
        load_catalog();
    }
}



void Catalog::save_catalog(){

    //fetch the first page of the catalog, then save my meta-data into it
    char* page_buffer = catalog_BPM->fetchPage(first_pag_id);

    int offset{};
    
    //save first_page_id
    memcpy(page_buffer+offset,&first_pag_id, sizeof(first_pag_id));
    offset+=sizeof(first_pag_id);

    //save number of tables on my catalog: 
    int num_of_tables = tables_ids_map.size();
    memcpy(page_buffer+offset,&num_of_tables, sizeof(num_of_tables));
    offset+=sizeof(num_of_tables);

    //save last table_id 
    memcpy(page_buffer+offset,&lst_table_id, sizeof(lst_table_id));
    offset+=sizeof(lst_table_id);

    catalog_BPM->mark_as_dirty(first_pag_id);

}

void Catalog::load_catalog(){

    //laod the first page in the file : 
    char* page_buffer = catalog_BPM->fetchPage(1);
    
    int offset{};

    //load first_page_id
    memcpy(&first_pag_id, page_buffer+offset, sizeof(first_pag_id));
    offset+=sizeof(first_pag_id);

    //load number of tables on my catalog: 
    int num_of_tables;
    memcpy(&num_of_tables,page_buffer+offset, sizeof(num_of_tables));
    offset+=sizeof(num_of_tables);

    //load last_table_id 
    memcpy(&lst_table_id, page_buffer+offset, sizeof(lst_table_id));
    offset+=sizeof(lst_table_id);

    //load the tables in my system : 
    this->tables_ids_map.clear();
    this->tables_names_map.clear();
    
    //each page after after the first page, holds a meta data of one table
    for(int i{first_pag_id}; i<=num_of_tables; i++){

        TableInfo* table_info = new TableInfo();
        //fetch each page, then load it's content: 
        char* page_buffer = catalog_BPM->fetchPage(i+1);
        table_info->load_table_info(DB_BPM, page_buffer);

        int table_id = table_info->get_table_id();
        string table_name = table_info->get_table_name();

        //add to our catalog
        tables_ids_map[table_id] = table_info;
        tables_names_map[table_name] = table_info;
    }
}


int TableInfo::save_table_info(char* buffer){
    int offset{0};

    //save table_name : 
    int table_name_length = table_name.size();
    memcpy(buffer+offset, &table_name_length, sizeof(table_name_length));
    offset += sizeof(table_name_length);

    //save the table_name : 
    memcpy(buffer + offset, table_name.data(), table_name.size());
    offset += table_name.size();


    //save table_id : 
    memcpy(buffer+offset, &table_id, sizeof(table_id));
    offset += sizeof(table_id);

    //save first_page_id : 
    memcpy(buffer+offset, &first_page_id, sizeof(first_page_id));
    offset += sizeof(first_page_id);
    
    //save schema: 

    //save schema size: 
    int schema_size = schema.size();
    memcpy(buffer+offset, &schema_size, sizeof(schema_size));
    offset += sizeof(schema_size);

    for(auto& col: schema){
        int col_size = col.serialize_col(buffer+offset);
        offset += col_size;
    }

    return offset;
}



int TableInfo::load_table_info(BufferPoolManager*DB_BPM, char* page_buffer){

    int offset{0};

    //load table_name : 
    int table_name_length;
    memcpy(&table_name_length, page_buffer+offset, sizeof(table_name_length));
    offset += sizeof(table_name_length);

    this->table_name.assign(page_buffer+offset, table_name_length);
    offset += table_name_length;

    //load table_id : 
    memcpy(&table_id, page_buffer+offset, sizeof(table_id));
    offset += sizeof(table_id);

    //load first_page_id : 
    memcpy(&first_page_id, page_buffer+offset, sizeof(first_page_id));
    offset += sizeof(first_page_id);
    
    //load schema: 

    //load schema size: 
    int schema_size;
    memcpy(&schema_size, page_buffer + offset, sizeof(schema_size));
    offset += sizeof(schema_size);

    this->schema.clear();

    for(int i{};i<schema_size;i++){
        Column col;
        int col_size = col.deserialize_col(page_buffer+ offset);
        offset += col_size;
        schema.push_back(col);
    }

    return offset;
}


TableInfo* Catalog::create_table(string table_name, vector<Column>schema){

    //check if table already exists: 
    if(tables_names_map.find(table_name) != tables_names_map.end()){
        throw runtime_error("table already exists");
    }

    //else, we create a new table 
    TableHeap* table_heap = new TableHeap(DB_BPM, schema, table_name, lst_table_id);

    //update our catalog

    int first_page_id = table_heap->get_first_page_id();
    TableInfo* table_info = new TableInfo(table_name, lst_table_id, schema, first_page_id);


    tables_ids_map[lst_table_id] = table_info;
    tables_names_map[table_name] = table_info;

    //save it to the file : 
    int page_id = catalog_BPM->newPage();
    char* page_buffer = catalog_BPM->fetchPage(page_id);

    table_info->save_table_info(page_buffer);
    catalog_BPM->mark_as_dirty(page_id);

    lst_table_id++;

    return table_info;
}


TableInfo* Catalog::get_table(string table_name){

    if(tables_names_map.find(table_name) != tables_names_map.end()){
        return tables_names_map[table_name];
    }
    return nullptr;
}


TableInfo* Catalog::get_table(int table_id){

    if(tables_ids_map.find(table_id) != tables_ids_map.end()){
        return tables_ids_map[table_id];
    }

    return nullptr;
}

int 
main(){

    DiskManager* catalog_dm = new DiskManager("catalog.bin");
    BufferPoolManager* catalog_BPM = new BufferPoolManager(catalog_dm);

    string file_name = "DB.bin";
    DiskManager* DB_dm = new DiskManager(file_name);
    BufferPoolManager* DB_BPM = new BufferPoolManager(DB_dm);

    bool is_new = false;
    Catalog* catalog = new Catalog(catalog_BPM, DB_BPM, is_new);

    if(is_new == true){

        string table_name = "User";

        vector<Column> schema;
        schema.push_back(Column(FieldType::TYPE_INT, "id", sizeof(int)));
        schema.push_back(Column(FieldType::TYPE_DOUBLE, "grade", sizeof(double)));
        schema.push_back(Column(FieldType::TYPE_STRING, "name", 50 ));

        cout<<"testing create table "<<endl;

        {
            TableInfo* table_info =  catalog->create_table(table_name, schema);

            cout<<"table id : "<< table_info->get_table_id()<<endl;
            cout<<"table name : "<< table_info->get_table_name()<<endl;
        }

        cout<<"testing get table "<<endl;

        {
            TableInfo* table_info =  catalog->get_table(table_name);

            cout<<"table id : "<< table_info->get_table_id()<<endl;
            cout<<"table name : "<< table_info->get_table_name()<<endl;
        }


    }

    else{

        string table_name = "User";

        cout<<"testing get table "<<endl;
        {
            TableInfo* table_info =  catalog->get_table(table_name);

            cout<<"table id : "<< table_info->get_table_id()<<endl;
            cout<<"table name : "<< table_info->get_table_name()<<endl;
        }
    }



    //save our data
    catalog->save_catalog();
    catalog_BPM->~BufferPoolManager();
    DB_BPM->~BufferPoolManager();

}
