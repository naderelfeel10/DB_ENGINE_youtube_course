#include"Storage/Table/TableHeap.h"
#include"Storage/Table/TableIterator.h"



TableHeap::TableHeap(BufferPoolManager* BPM,vector<Column> schema, string table_name, int table_id){
    
    //set the meta data:
    this->schema = schema;
    this->table_id = table_id;
    this->table_name = table_name;

    this->BPM = BPM;

    //then create the first page in the table 
    
    first_page_id =  BPM->newPage();
    last_page_id = first_page_id;

    this->starting_rid = RID(first_page_id, 0);

    cout<<table_name+"'s first page id : "<<first_page_id<<endl;
    cout<<table_name+"'s last page id : "<<last_page_id<<endl;
}


// reopen a table and find its last page and row
TableHeap::TableHeap(BufferPoolManager* BPM, vector<Column> schema, string table_name,int table_id, int first_page_id):table_name(table_name), table_id(table_id), schema(schema),
      first_page_id(first_page_id), last_page_id(first_page_id),
      starting_rid(first_page_id, 0), stopping_rid(-1, -1), BPM(BPM){

    // follow the page chain to find where new rows should be inserted
    while(true){

        char* data = BPM->fetchPage(last_page_id);
        PageHeader* header = reinterpret_cast<PageHeader*>(data);

        num_of_tuples += header->num_of_tuples;
        int slots = header->num_of_tuples + header->num_of_deleted_tuples;

        if(slots > 0){
            stopping_rid = RID(last_page_id, slots - 1);
        }

        if(header->next_page_id == -1){
            break;
        }

        last_page_id = header->next_page_id;
    }
}

bool TableHeap::check_schema(Tuple tuple){

    //check if fields types match : 
    vector<FieldType>fields_types;
    for(auto& f: tuple.get_fields()){
        fields_types.push_back(f.get_field_type());
    }

    if(fields_types.size() != schema.size()){
        return false;
    }

    for(int i{};i<schema.size();i++){
        if(this->schema[i].get_field_type() != fields_types[i]){
            return false;
        }
    }
    return true;
}   

RID TableHeap::insert_tuple(Tuple tuple){


    //check for the tuple schema first : 
    bool is_schema_valid = check_schema(tuple);


    if(is_schema_valid == false){
        throw runtime_error("tuple schemas don't match");
    }
    // reject rows that can never fit in an empty page
    int padded_size = (tuple.get_tuple_size() + 7) & ~7;
    if(padded_size + sizeof(Slot) + sizeof(PageHeader) > PAGE_SIZE){
        throw runtime_error("tuple is too large for a page");
    }

    //fetch the last page 
    //then we try to insert : 
        // if no space : 
        // create a new page 

    //fetch the last_page
    char* page_buffer = BPM->fetchPage(last_page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);
    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer);

    //try to insert : 
    int slot_num = page->insert_tuple(tuple);

    //check if the page has space : 
    if(slot_num == -1){
        //no space, then allocate new page : 
        int new_page_id = BPM->newPage();

        // allocating can evict the old page, so fetch it again
        page_buffer = BPM->fetchPage(last_page_id);
        page_header = reinterpret_cast<PageHeader*>(page_buffer);

        page_header->next_page_id = new_page_id;

        BPM->mark_as_dirty(last_page_id);

        this->last_page_id = new_page_id;

        //try to insert again : 
        RID rid = this->insert_tuple(tuple);
        return rid;
    }
    
    //if we have space 
    BPM->mark_as_dirty(last_page_id);
    this->num_of_tuples++;

    stopping_rid = RID(last_page_id, slot_num);

    // remove the new row and any added entries if an index insert fails
    vector<pair<BPLusTree*, Field>> attempted;
    try {
    for(auto&[col_name,index]:this->pplusindexes){

        int col_index{0};
        for(auto& c:schema){
            if(col_name == c.get_col_name()){
                break;
            }
            col_index++;
        }
        Field key = tuple.get_fields()[col_index];
        attempted.push_back({index, key});
        index->insert(key,stopping_rid);
    }
    }
     catch(...){
        for(auto& entry : attempted){
            entry.first->remove(entry.second, stopping_rid);
        }
        char* data = BPM->fetchPage(stopping_rid.get_page_id());

        reinterpret_cast<Page*>(data)->delete_tuple(stopping_rid.get_slot_num());
        BPM->mark_as_dirty(stopping_rid.get_page_id());
        --num_of_tuples;
        throw;
    }

    return stopping_rid;
}

Tuple* TableHeap::get_tuple(RID rid){

    //page_id, slot_nuum
    //fetch the page using page_id
    //then find the tuple with the slot num inside this page

    int page_id = rid.get_page_id();
    int slot_num = rid.get_slot_num();
    
    char* page_buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    Tuple* tuple = new Tuple({});
    bool is_found =  page->get_tuple(slot_num, *tuple);

    // return nothing when the slot is missing or deleted
    if(!is_found){
        delete tuple;
        return nullptr;
    }
    return tuple;
}

// replace a row and update its indexes through insert and delete
RID TableHeap::update_tuple(RID rid, Tuple new_tuple){
    if(!check_schema(new_tuple)){
        throw runtime_error("tuple schemas don't match");
    }
    Tuple* old_tuple = get_tuple(rid);
    if(old_tuple == nullptr){
        throw runtime_error("tuple not found");
    }
    delete old_tuple;

    // insert first so a failed insert leaves the old row in place
    RID new_rid = insert_tuple(new_tuple);
    if(!delete_tuple(rid)){
        delete_tuple(new_rid);
        throw runtime_error("could not delete old tuple during update");
    }
    return new_rid;
}

// remove a row and its entries from all attached indexes
bool TableHeap::delete_tuple(RID rid){
    Tuple* tuple = get_tuple(rid);
    if(tuple == nullptr){
        return false;
    }

    // remember removed entries so they can be restored if a later removal fails
    vector<pair<BPLusTree*, Field>> removed;
    for(auto& [col_name, index] : pplusindexes){
        size_t column = 0;
        while(column < schema.size() && schema[column].get_col_name() != col_name){
            ++column;
        }
        if(column == schema.size()){
            delete tuple;
            throw runtime_error("indexed column not found");
        }
        Field key = tuple->get_fields()[column];
        if(!index->remove(key, rid)){
            for(auto& entry : removed){
                entry.first->insert(entry.second, rid);
            }
            delete tuple;
            throw runtime_error("tuple missing from index");
        }
        removed.push_back({index, key});
    }
    delete tuple;

    // mark the row deleted after its index entries have been removed
    char* page_buffer = BPM->fetchPage(rid.get_page_id());
    Page* page = reinterpret_cast<Page*>(page_buffer);
    if(!page->delete_tuple(rid.get_slot_num())){
        for(auto& entry : removed){
            entry.first->insert(entry.second, rid);
        }
        return false;
    }
    BPM->mark_as_dirty(rid.get_page_id());
    --num_of_tuples;
    return true;
}

void TableHeap::addIndex(BPLusTree* index, string col_name){

    //add it to the map
    attachIndex(index, col_name);
    //fill the index
    fill_index(col_name);
}

// attach an index that is already built for this column
void TableHeap::attachIndex(BPLusTree* index, string col_name){

    size_t column = 0;

    while(column < schema.size() && schema[column].get_col_name() != col_name){
        ++column;
    }

    if(column == schema.size()){
        throw runtime_error("indexed column not found");
    }

    if(index == nullptr || pplusindexes.count(col_name)){
        throw runtime_error("invalid or duplicate index");
    }
    pplusindexes[col_name] = index;
}

BPLusTree* TableHeap::getIndex(string col_name){

    // find the index 
    auto it = pplusindexes.find(col_name);
    return it == pplusindexes.end() ? nullptr : it->second;
}

void TableHeap::fill_index(string col_name){
    //fetch the index first 
    BPLusTree* index  = this->getIndex(col_name);

    TableIterator it = TableIterator(this, starting_rid, stopping_rid);

    int col_index{0};
    for(auto& c:schema){
        if(col_name == c.get_col_name()){
            break;
        }
        col_index++;
    }

    for(; !it.end();++it){
        Tuple t = *it;  

        Field key = t.get_fields()[col_index];
        RID rid = it.get_curr_rid();
        index->insert(key, rid);
    }

}
/*
int 
main(){

    const string file_name = "DB.bin";
    DiskManager* dm = new DiskManager(file_name);

    BufferPoolManager* BPM = new BufferPoolManager(dm);


    string table_name = "User";
    int table_id = 1;

    vector<Column> schema;
    schema.push_back(Column(FieldType::TYPE_INT, "id", sizeof(int)));
    schema.push_back(Column(FieldType::TYPE_DOUBLE, "grade", sizeof(double)));
    schema.push_back(Column(FieldType::TYPE_STRING, "name", 50 ));


    TableHeap* table_heap = new TableHeap(BPM, schema, table_name, table_id);


    vector<Field>fields;
    {
    {
        FieldType type = FieldType::TYPE_INT;
        int value = 9;

        Field f(type, value);

        fields.push_back(f);
    }
    {
        FieldType type = FieldType::TYPE_DOUBLE;
        double value = 9.1234;

        Field f(type, value);
        fields.push_back(f);
        
    }
    {
        FieldType type = FieldType::TYPE_STRING;
        const char* value = "ELFEEL";

        Field f(type, value);
        fields.push_back(f);
        
    }
    }
        //create the tuple :
    Tuple tuple(fields);


    cout<<"testing insert tuple to a table"<<endl;
    {
        RID rid = table_heap->insert_tuple(tuple);
        rid.print();

        rid = table_heap->insert_tuple(tuple);
        rid.print();
    }
    cout<<"stress testing insert tuple to a table"<<endl;
    {
        int i{};
        while(i++<100){
            RID rid = table_heap->insert_tuple(tuple);
            rid.print();
        }
    }

    cout<<"testing get tuple from the rid "<<endl;
    {
        RID rid = RID(2, 32);
        Tuple* t = table_heap->get_tuple(rid);
        t->print_tuple();
    }

    cout<<"testing update tuple"<<endl;
    {
        fields[2] = Field(FieldType::TYPE_STRING, "NADER");
        Tuple new_tuple = Tuple(fields);
        RID rid = RID(2, 32);
        RID new_rid = table_heap->update_tuple(rid, new_tuple);

        Tuple* t = table_heap->get_tuple(new_rid);
        t->print_tuple();
    }
    
    cout<<"testing delete tuple from the rid "<<endl;
    {
        RID rid = RID(2, 32);
        bool res = table_heap->delete_tuple(rid);

        Tuple* t = table_heap->get_tuple(rid);
        t->print_tuple();
    }

}
*/
