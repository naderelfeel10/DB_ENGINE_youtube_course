#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"



// start at the first live row from the requested position
TableIterator::TableIterator(TableHeap* table_heap, RID starting_rid, RID stopping_rid)
    : curr_rid(starting_rid), stopping_rid(stopping_rid), table_heap(table_heap){
    skip_deleted();
}

// move through slots and pages until a live row is found
void TableIterator::skip_deleted(){
    while(curr_rid.get_page_id() != -1){
        int page_id = curr_rid.get_page_id();
        char* data = table_heap->BPM->fetchPage(page_id);
        PageHeader* header = reinterpret_cast<PageHeader*>(data);
        int slots_count = header->num_of_tuples + header->num_of_deleted_tuples;
        int slot = curr_rid.get_slot_num();
        Slot* slots = reinterpret_cast<Slot*>(data + sizeof(PageHeader));
        while(slot < slots_count && slots[slot].is_deleted){
            ++slot;
        }
        if(slot < slots_count){
            curr_rid = RID(page_id, slot);
            return;
        }
        // move to the next page when this one has no more live rows
        curr_rid = RID(header->next_page_id, 0);
    }
}

TableIterator& TableIterator::operator++(){

    //whend i call ++, advance the pointer to point the next tuple in the table 
    //fetch page : 
    int page_id = curr_rid.get_page_id();
    int next_slot_num = curr_rid.get_slot_num() + 1;
    curr_rid = RID(page_id, next_slot_num);
    
    skip_deleted();
    return *this;

}

Tuple TableIterator::operator*(){
    //fetch the tuple in the curr_page_rid
    Tuple* tuple = this->table_heap->get_tuple(curr_rid);
    Tuple t = *tuple;

    delete tuple;
    return t;
}

bool TableIterator::end(){
    
    int page_id = this->curr_rid.get_page_id();
    return page_id == -1;
}

/*
int main(){

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


    cout<<"testing table_iterator"<<endl;
    TableIterator it = TableIterator(table_heap, RID(1,0), table_heap->get_last_rid());

    {
        for(; !it.end();++it){
            Tuple t = *it;
            t.print_tuple();
        }
    }
}
*/
