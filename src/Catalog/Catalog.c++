#include"Catalog/Catalog.h"


Catalog::Catalog(BufferPoolManager* catalog_BPM ,BufferPoolManager* DB_BPM ,bool create_new){  

    this->catalog_BPM = catalog_BPM;
    this->DB_BPM = DB_BPM;

    //if we need to create a new catalog
    if(create_new == true){
        //allocate a page for the catalog
        first_page_id = catalog_BPM->newPage();
        lst_page_id = first_page_id;
        
        save_catalog();
    }
    //load the catalog
    else{
        load_catalog();
    }
}


TableInfo* Catalog::create_table(string table_name, vector<Column>schema){

    //check if the table already exists : 
    if(tables_names_map.find(table_name) != tables_names_map.end()){
        throw runtime_error("table with this name already exists");
    }

    //create the table : 
    TableHeap* table_heap = new TableHeap(DB_BPM,schema, table_name, lst_table_id);

    //create table_info
    int first_page_id = table_heap->get_first_page_id();
    TableInfo* table_info = new TableInfo(table_heap, table_name, lst_table_id, schema, first_page_id);


    //add table to catalog system
    tables_names_map[table_name] = table_info;
    tables_ids_map[lst_table_id] = table_info;

    //save this table to the catalog buffer : 

    int page_id = catalog_BPM->newPage();
    char* page_buffer = catalog_BPM->fetchPage(page_id);

    table_info->serializeTableInfo(page_buffer);
    catalog_BPM->mark_as_dirty(page_id);

    lst_table_id++;

    return table_info;
}



void Catalog::save_catalog(){

        //save table_meta data : 
        char* page_buffer = catalog_BPM->fetchPage(first_page_id);

        int offset{};

        memcpy(page_buffer + offset, &first_page_id, sizeof(first_page_id));
        offset += sizeof(first_page_id);

        memcpy(page_buffer + offset, &lst_page_id, sizeof(lst_page_id));
        offset += sizeof(lst_page_id);

        memcpy(page_buffer + offset, &lst_table_id, sizeof(lst_table_id));
        offset += sizeof(lst_table_id);

        int num_of_tables = this->get_number_of_tables();
        memcpy(page_buffer + offset, &num_of_tables, sizeof(num_of_tables));
        offset += sizeof(num_of_tables);

        catalog_BPM->mark_as_dirty(first_page_id);

        char* page_buffer2 = catalog_BPM->fetchPage(1);

}
void Catalog::load_catalog(){
    
    //first load catalog meta : 
    char* page_buffer = catalog_BPM->fetchPage(1);

    int offset{};
    //load first_page_id
    memcpy(&first_page_id, page_buffer + offset, sizeof(first_page_id));
    offset += sizeof(first_page_id);
    
    cout<<"first page_id : "<<first_page_id<<endl;

    //load last_page_id
    memcpy(&lst_page_id, page_buffer + offset, sizeof(lst_page_id));
    offset += sizeof(lst_page_id);

    cout<<"lst page_id : "<<lst_page_id<<endl;

    //load last_table_id
    memcpy(&lst_table_id, page_buffer + offset, sizeof(lst_table_id));
    offset += sizeof(lst_table_id);

    cout<<"lst table_id : "<<lst_table_id<<endl;

    //load num_of_tables
    int num_of_tables = this->get_number_of_tables();
    memcpy(&num_of_tables, page_buffer + offset, sizeof(num_of_tables));
    offset += sizeof(num_of_tables);

    cout<<"num_of_tables : "<<num_of_tables<<endl;

    //load tables : 
    this->tables_ids_map.clear();

    for(int i{first_page_id};i<=num_of_tables;i++){

        TableInfo* table_info = new TableInfo();

        //fetch the page the table allocated in :
        char* page_buffer = catalog_BPM->fetchPage(i+1);
        table_info->deserialize_tableInfo(DB_BPM ,page_buffer);

        int table_id = table_info->get_table_id();
        string table_name = table_info->get_table_name();
        
        cout<<"table id : "<< table_id<<endl;
        cout<<"table name : "<< table_name<<endl;

        //add to the catalog
        tables_ids_map[table_id] = table_info;
        tables_names_map[table_name] = table_info;
    }

}

int TableInfo::serializeTableInfo(char* buffer){

    int offset = 0;

    // save table_name length
    int table_name_size = table_name.length();
    memcpy(buffer+offset, &table_name_size, sizeof(table_name_size));
    offset += sizeof(table_name_size);

    //save table_name
    memcpy(buffer+offset, table_name.data(), table_name.size());
    offset += table_name.size();

    //save table_id
    memcpy(buffer+offset, &table_id, sizeof(table_id));
    offset += sizeof(table_id);

    //save first page_id of the table
    memcpy(buffer+offset, &first_page_id, sizeof(first_page_id));
    offset += sizeof(first_page_id);

    //save number of cols in schema
    int schema_size = schema.size();
    memcpy(buffer+offset, &schema_size, sizeof(schema_size));
    offset += sizeof(schema_size);

    for(auto&col : schema){
        int col_size = col.serialize_col(buffer+offset);
        offset += col_size;
    }

    return offset;

}

int TableInfo::deserialize_tableInfo(BufferPoolManager* DB_BPM, const char* buffer){

    int offset = 0;

    // read table_name length
    int table_name_size;
    memcpy(&table_name_size, buffer + offset, sizeof(table_name_size));
    offset += sizeof(table_name_size);

    cout<<"table name size " <<table_name_size<<endl;

    // read table_name
    this->table_name.assign(buffer + offset, table_name_size);
    offset += table_name_size;

    cout<<"table name  " <<table_name<<endl;

    // read table_id
    memcpy(&this->table_id, buffer + offset, sizeof(this->table_id));
    offset += sizeof(this->table_id);

    cout<<"table_id " <<table_id<<endl;

    // read first_page_id
    memcpy(&this->first_page_id, buffer + offset, sizeof(this->first_page_id));
    offset += sizeof(this->first_page_id);

    cout<<"first page id  " <<first_page_id<<endl;

    // read number of columns
    int schema_size;
    memcpy(&schema_size, buffer + offset, sizeof(schema_size));
    offset += sizeof(schema_size);

    cout<<"schema size  " <<schema_size<<endl;

    // read columns
    this->schema.clear();
    
    for(int i = 0; i < schema_size; i++){
        Column col;

        int col_size =col.deserialize_col(buffer + offset);
        offset += col_size;
        this->schema.push_back(col);
        col.print();
    }

    this->table_heap =  new TableHeap(DB_BPM, schema, table_name, table_id);

    return offset;
}


int 
main(){

    DiskManager* catalog_dm = new DiskManager("catalog.db");
    BufferPoolManager* catalog_BPM = new BufferPoolManager(catalog_dm);

    const string file_name = "DB.bin";
    DiskManager* DB_dm = new DiskManager(file_name);

    BufferPoolManager* DB_BPM = new BufferPoolManager(DB_dm);


    string table_name = "User";
    int table_id = 1;

    vector<Column> schema;
    schema.push_back(Column(FieldType::TYPE_INT, "id", sizeof(int)));
    schema.push_back(Column(FieldType::TYPE_DOUBLE, "grade", sizeof(double)));
    schema.push_back(Column(FieldType::TYPE_STRING, "name", 50 ));


    bool new_catalog = false;
    Catalog* catalog = new Catalog(catalog_BPM, DB_BPM, new_catalog);

    if(new_catalog == true){

    cout<<"testing create table using catalog"<<endl;

    {
        TableInfo* table_info =  catalog->create_table(table_name, schema);

        assert(table_info != nullptr);
        assert(table_info->get_table_name() == table_name);
        assert(table_info->get_table_heap() != nullptr);
        
    }

    cout<<"testing get table using name and id"<<endl;
    {
        TableInfo* table_info =  catalog->get_table(table_name);

        assert(table_info != nullptr);
        assert(table_info->get_table_name() == table_name);
        assert(table_info->get_table_heap() != nullptr);
    }

    }

    else{
        string table_name = "User";
        TableInfo*  table_info = catalog->get_table(table_name);
        
        assert(table_info != nullptr);
        assert(table_info->get_table_name() == table_name);
        assert(table_info->get_table_heap() != nullptr);
    }


    catalog->save_catalog();
    catalog_BPM->~BufferPoolManager();
    DB_BPM->~BufferPoolManager();


}