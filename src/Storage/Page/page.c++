#include"Storage/Page/Page.h"

Page::Page(int page_id){
    //clear this page 
    memset(data , 0, PAGE_SIZE);
    
    //intialize our header : 
    PageHeader* header = reinterpret_cast<PageHeader*>(data);

    header->page_id = page_id;
    header->num_of_deleted_tuples = 0;
    header->num_of_tuples = 0;
    header->offset_pointer = PAGE_SIZE;

}

int Page::get_page_id(){

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    return header->page_id;
}   


int Page::insert_tuple(Tuple tuple){

    // check if there is enough space
    int tuple_size = tuple.get_tuple_size();
    tuple_size = (tuple_size+7)& ~7; // padding 

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(header == nullptr){
        throw runtime_error("header is nullptr");
    }

    int num_of_slots = header->num_of_tuples;
    int slot_size = sizeof(Slot);
    int free_space = header->offset_pointer - (sizeof(PageHeader)  + slot_size*num_of_slots);

    //check for space againtest new tuple
    if(free_space < (tuple_size + slot_size)){
        cout<<"no enough space in this page "<<header->page_id<<endl;
        return -1;
    }

    //else means there is space to insert our tuple :

    // serialize the tuple first
    header->offset_pointer -= tuple_size;
    tuple.serialize_tuple(data + header->offset_pointer);

    //create a slot that points at this tuple 
    Slot* slots = reinterpret_cast<Slot*>(data + sizeof(PageHeader));
    slots[num_of_slots].offset = header->offset_pointer;
    slots[num_of_slots].size = tuple_size;

    header->num_of_tuples ++;

    //return the slot number 

    return (header->num_of_tuples - 1);    
}

bool Page::get_tuple(int slot_num, Tuple& tuple){

    //then access the slot, then fetch the tuple from it's offset
    //then deserialize this tuple 

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(header == nullptr){
        throw runtime_error("header is nullptr");
    }

    Slot* slots = reinterpret_cast<Slot*>(data + sizeof(PageHeader));
    
    int offset = slots[slot_num].offset;
    int size = slots[slot_num].size;

    //load the tuple from the page
    tuple.deserialize_tuple(data + offset);

    if(tuple.get_is_deleted() == true){
        cout<<"this tuple is deleted"<<endl;
        return false;
    }

    return true;

}

bool Page::delete_tuple(int slot_num){
    
    //then access the slot, then fetch the tuple from it's offset
    //then deserilize the tuple
    // mark it as delete
    //serialize it back into the page
    // then mark the slot as deleted

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(header == nullptr){
        throw runtime_error("header is nullptr");
    }

    //get tuple offset
    Slot* slots = reinterpret_cast<Slot*>(data + sizeof(PageHeader));
    
    int offset = slots[slot_num].offset;
    int size = slots[slot_num].size;

    //mark this tuple as deleted
    Tuple dummy_tuple({});
    
    dummy_tuple.deserialize_tuple(data + offset);
    
    dummy_tuple.mark_as_deleted();
    
    dummy_tuple.serialize_tuple(data + offset);

    //mark the slot as delete
    slots[slot_num].is_deleted =  true;
    header->num_of_deleted_tuples ++;
    header->num_of_tuples --;

    return true;

}

int Page::update_tuple(int slot_num, Tuple new_tuple){


    // fetch the old tuple
    // compare old and new tuple sizes
    //if new > old, then delete and reinsert
    //else, then just overwrite the old one 

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(header == nullptr){
        throw runtime_error("header is nullptr");
    }

    //get tuple offset
    Slot* slots = reinterpret_cast<Slot*>(data + sizeof(PageHeader));
    
    int offset = slots[slot_num].offset;
    
    int old_tuple_size = slots[slot_num].size;
    int new_tuple_size = new_tuple.get_tuple_size();

    if(new_tuple_size > old_tuple_size){
        this->delete_tuple(slot_num);
        int new_slot_number = this->insert_tuple(new_tuple);
        return new_slot_number;
    }

    new_tuple.serialize_tuple(data + offset);

    return slot_num;

}


/*
int 
main(){

    vector<Field>fields;

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

    //create the tuple :
    Tuple tuple(fields);


    {
        Page page(5);
        cout<<"page id : "<<page.get_page_id()<<endl;
        assert(5 == page.get_page_id());

        int slot_num = page.insert_tuple(tuple);
        int slot_num2 = page.insert_tuple(tuple);

        tuple.print_tuple();
        cout<<endl;

        Tuple new_tuple({});
        page.get_tuple(slot_num, new_tuple);
        new_tuple.print_tuple();
        cout<<endl;

    }

    cout<<"testing delete tuple"<<endl;
    {

        Page page(5);
        cout<<"page id : "<<page.get_page_id()<<endl;
        assert(5 == page.get_page_id());

        int slot_num = page.insert_tuple(tuple);
        tuple.print_tuple();
        cout<<endl;

        page.delete_tuple(slot_num);

        Tuple new_tuple({});
        page.get_tuple(slot_num, new_tuple);
        new_tuple.print_tuple();
        cout<<endl;

    }


    cout<<"testing update tuple"<<endl;
    {

        Page page(5);
        cout<<"page id : "<<page.get_page_id()<<endl;
        assert(5 == page.get_page_id());

        int slot_num = page.insert_tuple(tuple);
        tuple.print_tuple();
        cout<<endl;

        Field f(FieldType::TYPE_BOOL, true);
        fields.push_back(f);

        Tuple updated_tuple(fields);
        int new_slot_num  = page.update_tuple(slot_num, updated_tuple);

        Tuple new_tuple({});
        page.get_tuple(new_slot_num, new_tuple);
        new_tuple.print_tuple();
        cout<<endl;

    }
}*/