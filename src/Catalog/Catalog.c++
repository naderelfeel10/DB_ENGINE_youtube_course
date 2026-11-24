#include"Catalog/Catalog.h"
#include"Storage/Table/TableIterator.h"
#include<cstdint>


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



// the marker separates the page directory from the older catalog format
static const int catalog_format = 0x43415432;
static const int index_format = 0x494e4432;

// save the catalog header and table metadata page ids
void Catalog::save_catalog(){

    if(table_pages.size() > (PAGE_SIZE - 4 * sizeof(int)) / sizeof(int)){
        throw runtime_error("catalog has too many tables");
    
    }
    //fetch teh page contains catalog meta data
    char* buffer = catalog_BPM->fetchPage(first_pag_id);

    int offset = 0;
    int count = table_pages.size();
    
    //save first page id 
    memcpy(buffer +offset, &first_pag_id, sizeof(int)); 
    offset += sizeof(int);

    //save number of pages in teh catalog
    memcpy(buffer + offset, &count, sizeof(int)); 
    offset += sizeof(int);

    //save last page id 
    memcpy(buffer + offset, &lst_table_id, sizeof(int)); 
    offset += sizeof(int);

    //save the format of the catalog 
    memcpy(buffer + offset, &catalog_format, sizeof(int)); 
    offset += sizeof(int);

    //then save all pages ids 
    for(int page_id : table_pages){
        memcpy(buffer + offset, &page_id, sizeof(int));
        offset += sizeof(int);
    }
    catalog_BPM->mark_as_dirty(first_pag_id);
}

// reopen the catalog, tables, and saved indexes
void Catalog::load_catalog(){

    //loading will always start from page 1
    char* buffer = catalog_BPM->fetchPage(1);
    
    int offset = 0;
    int number_of_tables = 0;
    int format = 0;
    
    //load first page id 
    memcpy(&first_pag_id, buffer + offset, sizeof(int)); 
    offset += sizeof(int);

    //load number of pages in teh catalog
    memcpy(&number_of_tables, buffer + offset, sizeof(int)); 
    offset += sizeof(int);

    //save last page id 
    memcpy(&lst_table_id, buffer + offset, sizeof(int)); 
    offset += sizeof(int);

    //save the format of the catalog 
    memcpy(&format, buffer + offset, sizeof(int)); 
    offset += sizeof(int);

    if(first_pag_id != 1||number_of_tables < 0){
        throw runtime_error("invalid catalog header");
    }

    //just a check to decide it's the correct catalog file
    if(format != 0 && format != catalog_format){
        throw runtime_error("unknown catalog format");
    }


    //load table pages and the maps from teh catalog
    table_pages.clear();
    tables_ids_map.clear();
    tables_names_map.clear();

    // older catalogs use consecutive pages, newer ones save the page ids
    for(int i = 0; i < number_of_tables; ++i){

        //initialize with number greater than 1
        int page_id{2};

        if(format == catalog_format){
            memcpy(&page_id, buffer + offset, sizeof(int));
            offset += sizeof(int);
        }

        if(page_id <= 1){
            throw runtime_error("invalid table metadata page");
        }

        table_pages.push_back(page_id);
    }

    // reopen each table and attach its saved indexes
    for(int page_id : table_pages){

        TableInfo* table = new TableInfo();

        table->metadata_page_id = page_id;

        //load the table from it's first_pageid
        char* data = catalog_BPM->fetchPage(page_id);
        //this load table returns the offset 
        int position = table->load_table_info(DB_BPM, data);

        //create the heap in the mem then assign it to the table
        table->heap = new TableHeap(DB_BPM, table->schema, table->table_name, table->table_id, table->first_page_id);

        int marker = 0;

        if(position + static_cast<int>(2 * sizeof(int)) <= PAGE_SIZE){
            memcpy(&marker, data + position, sizeof(int));
        }

        //now load the indexes of this table
        if(marker == index_format){

            //first load number of indexes
            position += sizeof(int);
            int index_count = 0;
            memcpy(&index_count, data + position, sizeof(int)); position += sizeof(int);

            if(index_count < 0 || index_count > PAGE_SIZE / static_cast<int>(4 * sizeof(int))){
                throw runtime_error("invalid index count");
            }

            //load each index then add it ot the tabl
            for(int i = 0; i < index_count; i++){
            
                IndexInfo* index = new IndexInfo();
            
                //read index name
                int name_length;
                
                ////read the length first
                memcpy(&name_length, data + position, sizeof(int));
                position += sizeof(int);
                
                //then read the actual name
                index->name = string(data + position, name_length);
                position += name_length;
            
                //read col_name
                int column_length;
                
                //read len
                memcpy(&column_length, data + position, sizeof(int));
                position += sizeof(int);
                
                //read the name itself
                index->column_name = string(data + position, column_length);
                position += column_length;
            
                //load index dgree
                memcpy(&index->degree, data + position, sizeof(int));
                position += sizeof(int);
            
                //load index root id
                memcpy(&index->root_page_id, data + position, sizeof(int));
                position += sizeof(int);
        
                //create the index then add it to the table
                index->tree = new BPLusTree(DB_BPM,index->degree,index->root_page_id);
                // add index to table
                table->indexes.push_back(index);
                //connect index to heap and catalog
                attach_index(table, index);
            }

        }

        if(tables_ids_map.count(table->table_id) || tables_names_map.count(table->table_name)){
            throw runtime_error("duplicate table metadata");
        }

        //add to catalog maps
        tables_ids_map[table->table_id] = table;
        tables_names_map[table->table_name] = table;
    }
}

// save the table schema and its index metadata in one catalog page
void Catalog::save_table(TableInfo* table){
    
    // calculate the required space before writing to the page

    size_t size = 4 * sizeof(int) + table->table_name.size();
    
    //save col names inside the schema 
    for(auto& column : table->schema){
        size += 1 + sizeof(int) + column.get_col_name().size();
    }

    //save indexes of each table
    size += 2 * sizeof(int);
    for(IndexInfo* index : table->indexes){
        size += 4 * sizeof(int) + index->name.size() + index->column_name.size();
    }
    // just a check if it exceeds one page
    if(size > PAGE_SIZE){
        throw runtime_error("table metadata does not fit in a catalog page");
    }
    //then save it
    char* buffer = catalog_BPM->fetchPage(table->metadata_page_id);
    
    //save table info then return the size it required
    int offset = table->save_table_info(buffer);
    
    memcpy(buffer + offset, &index_format, sizeof(int));
    offset += sizeof(int);
    
    //save 
    int num_of_indexes = table->indexes.size();
    memcpy(buffer + offset, &num_of_indexes, sizeof(int));
    offset += sizeof(int);
    
    //then save each index 
    for(IndexInfo* index : table->indexes){

        //first save indexname and col name and their lengths for loading 
        for(const string* value : {&index->name, &index->column_name}){
            
            int length = value->size();
            //save the length
            memcpy(buffer + offset, &length, sizeof(int));
            offset += sizeof(int);
            //then save the value
            memcpy(buffer + offset, value->data(), length);
            offset += length;
        }
        //save the degree of the index (for b+ tree indexes)
        memcpy(buffer + offset, &index->degree, sizeof(int));
        offset += sizeof(int);
        //finally save teh root page id, from it we can load the full tree
        memcpy(buffer + offset, &index->root_page_id, sizeof(int)); 
        offset += sizeof(int);
    }
    

    catalog_BPM->mark_as_dirty(table->metadata_page_id);
}

// attach the index and save its root whenever it changes
void Catalog::attach_index(TableInfo* table, IndexInfo* index){

    //that is a call back function that is executed when ever the root changes so we save teh table meta data into the catalog
    index->tree->on_root_change = [this, table, index](int page_id){
        
        index->root_page_id = page_id;
        save_table(table);
    };
    //attatch the updated index to the table 
    table->heap->attachIndex(index->tree, index->column_name);
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

    if(table_name_length < 0 || table_name_length > PAGE_SIZE - offset - static_cast<int>(3 * sizeof(int))){
        throw runtime_error("invalid table name length");
    }

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

    if(schema_size < 0 || schema_size > (PAGE_SIZE - offset) / static_cast<int>(1 + sizeof(int))){
        throw runtime_error("invalid schema size");
    }

    this->schema.clear();

    for(int i{};i<schema_size;i++){

        if(offset + static_cast<int>(1 + sizeof(int)) > PAGE_SIZE){
            throw runtime_error("invalid column metadata");
        }

        int name_size = 0;
        memcpy(&name_size, page_buffer + offset + 1, sizeof(int));
        if(name_size < 0 || name_size > PAGE_SIZE - offset - static_cast<int>(1 + sizeof(int))){
            throw runtime_error("invalid column name length");
        }

        Column col;
        int col_size = col.deserialize_col(page_buffer+ offset);
        offset += col_size;
        schema.push_back(col);
    }

    return offset;
}


// create a table and save its metadata in the catalog
TableInfo* Catalog::create_table(string table_name, vector<Column> schema){

    if(tables_names_map.count(table_name)){
        throw runtime_error("table already exists");
    }

    size_t metadata_size = 6 * sizeof(int) + table_name.size();

    if(table_name.empty() || schema.empty()){
        throw runtime_error("table name and schema must not be empty");
    }

    for(size_t i = 0; i < schema.size(); ++i){

        if(schema[i].get_col_name().empty()){
            throw runtime_error("column name must not be empty");
        }

        for(size_t j = 0; j < i; ++j){
            if(schema[i].get_col_name() == schema[j].get_col_name()){
                throw runtime_error("duplicate column name");
            }
        }
    }

    //calc shcema size
    for(auto& column : schema){
        metadata_size += 1 + sizeof(int) + column.get_col_name().size();
    }
    //check if exceeds PAGE_SIZE
    if(metadata_size > PAGE_SIZE){
        throw runtime_error("table metadata does not fit in a catalog page");
    }

    // create the heap and a catalog page that describes it
    TableHeap* heap = new TableHeap(DB_BPM, schema, table_name, lst_table_id);
    TableInfo* table = new TableInfo(table_name, lst_table_id, schema, heap->get_first_page_id());

    table->heap = heap;
    table->metadata_page_id = catalog_BPM->newPage();

    save_table(table);
    
    tables_ids_map[lst_table_id] = table;
    tables_names_map[table_name] = table;
    
    // save the page id and next table id in the catalog header
    table_pages.push_back(table->metadata_page_id);
    
    ++lst_table_id;
    
    save_catalog();
    return table;
}

// find an index by its name
IndexInfo* Catalog::get_index(string index_name){

    for(auto& [id, table] : tables_ids_map){
        for(IndexInfo* index : table->indexes){
            
            if(index->name == index_name){
                return index;
            }
        }
    }

    return nullptr;
}

// build an index for a column and save its metadata
IndexInfo* Catalog::create_index(string index_name, string table_name, string column_name, int degree){

    TableInfo* table = get_table(table_name);

    if(table == nullptr){
        throw runtime_error("table not found");
    }

    if(index_name.empty() || get_index(index_name)){
        throw runtime_error("index name can't be empty");
    }
    if(get_index(index_name) != nullptr){
        throw runtime_error("duplicated index name");
    }
    
    //find the index of the col first
    size_t column_index = 0;
    while(column_index < table->schema.size()&& table->schema[column_index].get_col_name() != column_name){
        column_index++;
    }

    //check if the col is not found
    if(column_index == table->schema.size()){
        throw runtime_error("column not found");
    }
    //check if we already have index on this col
    if(table->heap->getIndex(column_name)){ 
        throw runtime_error("column already has an index");
    }

    // check that the table and index metadata fit in one catalog page
    size_t metadata_size = 6 * sizeof(int) + table->table_name.size();

    for(auto& col : table->schema){
        metadata_size += 1+sizeof(int)+col.get_col_name().size();
    }
    for(IndexInfo* existing : table->indexes){
        metadata_size += 4*sizeof(int)+existing->name.size()+existing->column_name.size();
    }

    metadata_size += 4 * sizeof(int) + index_name.size() + column_name.size();

    if(metadata_size > PAGE_SIZE){
        throw runtime_error("table metadata does not fit in a catalog page");
    }

    // add the existing rows to the new tree
    BPLusTree* new_btree = new BPLusTree(DB_BPM, degree, -1);

    IndexInfo* index = new IndexInfo{index_name, column_name, degree, -1, new_btree};

    //loop over all rows in the table and insert them into the new index
    TableIterator it(table->heap, RID(table->first_page_id, 0), table->heap->get_last_rid());
    for(; !it.end(); ++it){
        Tuple tuple = *it;
        index->tree->insert(tuple.get_fields()[column_index], it.get_curr_rid());
    }

    if(index->tree->root){
        index->root_page_id = index->tree->root->page_id;
    }
    
    // save and attach the index so future writes can update it
    table->indexes.push_back(index);
    
    try {
        //save table and attatch the new index
        save_table(table);
        attach_index(table, index);
    } catch(...){
        table->indexes.pop_back();
        delete index->tree;
        delete index;
        throw;
    }
    return index;
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
/*
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
*/
