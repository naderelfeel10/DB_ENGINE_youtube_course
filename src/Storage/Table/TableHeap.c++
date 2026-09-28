#include"Storage/Table/TableHeap.h"


TableHeap::TableHeap(BufferPoolManager* BPM, vector<Column>schema, string table_name, int table_id){

    this->BPM = BPM;
    this->schema = schema;
    this->table_name = table_name;
    this->table_id = table_id;

    //then allocate pages for the table :
    this->first_page_id = BPM->newPage();
    this->last_page_id = BPM->newPage();

    this->starting_rid = RID(first_page_id, 0);

    cout<<table_name + "'s first p_id : "<<this->first_page_id<<endl;
    cout<<table_name + "'s last p_id : "<<this->last_page_id<<endl;

}

bool TableHeap::check_schema(Tuple tuple){
    
    
    vector<FieldType> fields_types;

    for(auto& f: tuple.get_fields()){
        fields_types.push_back(f.get_field_type());
    }

    if(schema.size() != fields_types.size()){
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

    //first check the schema : 
    
    bool is_schema_matched = this->check_schema(tuple);

    if(is_schema_matched == false){
        throw runtime_error("schemas don't match");
    }

    //fetch the last page in our table: 
    //then try to insert
    //if no space then :
        //1. create new page
        //2. update last_page_id to the newly created page
    
    
    
    //fetch the last page : 
    char* last_page_buffer = this->BPM->fetchPage(last_page_id);
    Page* last_page = reinterpret_cast<Page*>(last_page_buffer);
    PageHeader* page_header = reinterpret_cast<PageHeader*>(last_page_buffer);

    int slot_num = last_page->insert_tuple(tuple);

    //if this slot num is -1, means no space left in this page,
    if(slot_num == -1){

        //create new page: 
        int new_page_id = BPM->newPage();
        page_header->next_page_id = new_page_id;

        BPM->mark_as_dirty(last_page_id);
        this->last_page_id = new_page_id;

        //try again with the new page
        
        RID lst_rid = this->insert_tuple(tuple);
        return lst_rid;
    }
    BPM->mark_as_dirty(last_page_id);
    
    this->num_of_tuples++;
    //cout<<"number of tuples : "<<num_of_tuples<<endl;

    //update stopping rid :
    this->stoping_rid = RID(last_page_id, slot_num);

    return stoping_rid;
    
}
Tuple* TableHeap::get_tuple(RID rid){
    //fetch the page, then use it to get the tuple
    int page_id = rid.get_page_id(); 
    int slot_num = rid.get_slot_num(); 

    cout<<page_id<<", "<<slot_num<<endl;

    char* page_buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    Tuple* tuple = new Tuple({});
    bool is_found = page->get_tuple(slot_num, *tuple);
    //tuple->print_tuple();
    return tuple;
}


RID TableHeap::update_tuple(RID rid, Tuple tuple){

    //first check the schema : 
    
    bool is_schema_matched = this->check_schema(tuple);

    if(is_schema_matched == false){
        throw runtime_error("schemas don't match");
    }

    //fetch the old tuple, compare sizes, the choose whether to overwrite or to delete and reinsert
    
    //fetch the page : 
    int page_id = rid.get_page_id();
    char* page_buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    int slot_num = rid.get_slot_num();
    //try to update
    int new_slot_num = page->update_tuple(slot_num, tuple);
    //if new slot num is -1, means : 
        // new tuple size > old tuple siz, so we have to delete the old one, then reinsert the new one
    if(new_slot_num == -1){
        RID new_rid = this->insert_tuple(tuple);
        return new_rid;
    }
    //if not, then I will just update the new slot num:
    RID new_rid = RID(page_id, new_slot_num);
    return new_rid;
}

bool TableHeap::delete_tuple(RID rid){

    //fetch the page, then delete it :
    int page_id = rid.get_page_id();
    char* page_buffer = BPM->fetchPage(page_id);

    Page* page = reinterpret_cast<Page*>(page_buffer);

    int slot_num = rid.get_slot_num();
    bool is_deleted = page->delete_tuple(slot_num);

    this->num_of_tuples--;

    return is_deleted;
}

int TableHeap::get_table_id(){
    return this->table_id;
}
string TableHeap::get_table_name(){
    return this->table_name;
}

void TableHeap::set_table_name(string new_name){
    this->table_name = new_name;
}


void TableHeap::delete_table(){

    //we have to delete all of it's pages from the file : 
    
    int curr_page_id = this->first_page_id;
    
    while (curr_page_id != -1){
        
        //fetch the page 
        //delete it
        //fetch it's next page in the linked list
        char* page_buffer = this->BPM->fetchPage(curr_page_id);
        //check if nullptr
        if(page_buffer == nullptr){
            throw runtime_error("page buffer is nullptr");
        } 

        PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
        int next_page_id = header->next_page_id;

        BPM->deletePage(curr_page_id);

        curr_page_id = next_page_id;
    }
    
}


int 
main(){

    const string file_name = "DB2.bin";
    DiskManager* dm = new DiskManager(file_name);

    BufferPoolManager* BPM = new BufferPoolManager(dm);

    string table_name = "User";
    int table_id = 1;
    
    vector<Column>schema;
    schema.push_back(Column(FieldType::TYPE_INT, "id", sizeof(int)));
    schema.push_back(Column(FieldType::TYPE_DOUBLE, "grade", sizeof(double)));
    schema.push_back(Column(FieldType::TYPE_STRING, "name", 30));


    TableHeap* table_heap = new TableHeap(BPM, schema, table_name, table_id);

    vector<Field>fields;
    //create dummy tuple
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

    cout<<"testing inserting 100 tuple"<<endl;
    int i{};
    while(i++<100){
        RID rid = table_heap->insert_tuple(tuple);
        rid.print();
        //Tuple* t1 = table_heap->get_tuple(rid);
        //t1->print_tuple();
    }

    cout<<"testing get tuple from rid"<<endl;
    {
        RID rid1 = RID(2,0);
        Tuple* t1 = table_heap->get_tuple(rid1);
        t1->print_tuple();
    }
    {
        RID rid1 = RID(2,12);
        Tuple* t1 = table_heap->get_tuple(rid1);
        t1->print_tuple();
    }
    {
        RID rid1 = RID(3,0);
        Tuple* t1 = table_heap->get_tuple(rid1);
        t1->print_tuple();
    }
    {
        RID rid1 = RID(3,12);
        Tuple* t1 = table_heap->get_tuple(rid1);
        t1->print_tuple();
    }
    {
        RID rid1 = RID(3,15);
        Tuple* t1 = table_heap->get_tuple(rid1);
        t1->print_tuple();
    }


    cout<<"testing update tuple "<<endl;
    {
        //fields.push_back(Field(FieldType::TYPE_BOOL, true));
        //fields.push_back(Field(FieldType::TYPE_STRING, "NADER"));

        fields[2] = Field(FieldType::TYPE_STRING, "NADER");
        Tuple new_tuple = Tuple(fields);
        RID rid = RID(3,10);
        
        RID new_rid = table_heap->update_tuple(rid, new_tuple);
        new_rid.print();

        Tuple* t1 = table_heap->get_tuple(new_rid);
        t1->print_tuple();

    }

    cout<<"testing delete tuple "<<endl;
    {

        RID rid = RID(3,10);
        
        table_heap->delete_tuple(rid);

        Tuple* t1 = table_heap->get_tuple(rid);
        t1->print_tuple();
    }


}
