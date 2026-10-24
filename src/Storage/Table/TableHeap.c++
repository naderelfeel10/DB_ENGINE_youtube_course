#include"Storage/Table/TableHeap.h"



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

    return tuple;
}

RID TableHeap::update_tuple(RID rid, Tuple new_tuple){
    
    //check for the tuple schema first : 
    bool is_schema_valid = check_schema(new_tuple);


    if(is_schema_valid == false){
        throw runtime_error("tuple schemas don't match");
    }

    //fetch the page : 
    int page_id = rid.get_page_id();
    int slot_num = rid.get_slot_num();
    
    char* page_buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    int new_slot_num = page->update_tuple(slot_num, new_tuple);

    //check if it's inserted into the old page 
    if(new_slot_num == -1){
        RID new_rid = this->insert_tuple(new_tuple);
        return new_rid;
    }

    //else : means we have updated it in the same old page ; 
    RID new_rid = RID(page_id, new_slot_num);
    return new_rid;
}
bool TableHeap::delete_tuple(RID rid){

    //fetch the page, then try to delete 
    int page_id = rid.get_page_id();
    int slot_num = rid.get_slot_num();
    
    char* page_buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    bool is_deleted =  page->delete_tuple(slot_num);

    this->num_of_tuples--;
    return is_deleted;
}


void TableHeap::addIndex(BPLusTree* index, string col_name){
    this->B_plus_indexes[col_name] = index;
    fill_index(col_name);
}

void TableHeap::fill_index(string col_name){

    BPLusTree* index = B_plus_indexes[col_name];

    TableIterator it = TableIterator(this, starting_rid, stopping_rid);

    int col_index = 0;
    for(auto& c : schema){
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