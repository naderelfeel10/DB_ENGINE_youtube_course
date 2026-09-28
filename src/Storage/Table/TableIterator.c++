#include"Storage/Table/TableIterator.h"


TableIterator& TableIterator::operator++(){
    //when i call this ++, i want it to return the next rid in the table
    
    //fetch the page in curr_rid
    int page_id = this->curr_rid.get_page_id();
    char* page_buffer = this->table_heap->BPM->fetchPage(page_id);

    PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    //prepare next page to fetch, and next slot to fetch : 
    int next_page_id = page_header->next_page_id;
    //next slot = curr_slot + 1
    int next_slot_num = curr_rid.get_slot_num() + 1;

    
    if(next_page_id == -1){
        curr_rid = RID(-1,-1);
        return *this;
    }

    //if the curr page is done, move to the next one
    if(next_slot_num >= page_header->num_of_tuples){
        this->curr_rid = RID(next_page_id, 0);
    }
    //else, so fetch the next slot in the same page
    else{
        curr_rid = RID(page_id, next_slot_num);
    }

    return *this;
    
}   

bool TableIterator::end(){
    int curr_page_id = curr_rid.get_page_id();
    return curr_page_id == -1;
}

// get the curr tuple 
Tuple TableIterator::operator*(){
    //fetch the tuple from the table
    Tuple* tuple = this->table_heap->get_tuple(curr_rid);
    
    Tuple t = *tuple;
    
    delete tuple;
    return t;
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
    }


    cout<<"tesging table iterator"<<endl;
    TableIterator it =  TableIterator(table_heap,RID(2,0), table_heap->get_stopping_rid());

    {
    for(; !it.end(); ++it){
        Tuple tuple = *it;
        tuple.print_tuple();
    }   
    }


}