#include"Storage/Page/Tuple.h"


Tuple::Tuple(vector<Field>fields){
    //loop over the fields, then push to our vector
    size = sizeof(is_deleted) + sizeof(size);
    for(auto& f : fields){
        this->fields.push_back(f);
        this->size += f.get_serialized_size();
    }

    this->is_deleted = false;
}



bool Tuple::get_is_deleted(){
    return this->is_deleted;
}

int Tuple::get_tuple_size(){
    int total_size = sizeof(this->is_deleted) + sizeof(this->size);

    for(auto& field : fields){
        total_size += field.get_serialized_size();
    }

    return total_size;
}


vector<Field> Tuple::get_tuple_fields(){
    return this->fields;
}


void Tuple::print_tuple(){
    cout<<"--------------"<<endl;
    cout<<(this->is_deleted?"DELETED":"FOUND")<<endl;

    // print each field
    for(auto&f : fields){
        f.print_field();
    }
    cout<<endl<<"--------------"<<endl;

}
void Tuple::mark_as_deleted(){
    this->is_deleted = true;
}


/*
    vector<Field>fields;

    //meta data : 
    bool is_deleted;
    int size;
*/

void Tuple::serialize_tuple(char* buffer){
    
    int offset{0};

    //serialize is_deleted 
    memcpy(buffer+offset, &this->is_deleted, sizeof(is_deleted));
    offset += sizeof(this->is_deleted);

    //serialize size 
    memcpy(buffer+offset, &this->size, sizeof(size));
    offset += sizeof(this->size);

    cout<<"sizeeee"<<endl;
    cout<<this->size<<endl;
    //serialize fields:
    for(auto&f :fields){
        f.serialize_field(buffer+offset);
        offset += f.get_serialized_size();
    }
 
}

void Tuple::deserialize_tuple(char* buffer){

    int offset {0};

    //load is_deleted
    memcpy(&this->is_deleted,buffer+offset, sizeof(is_deleted));
    offset += sizeof(this->is_deleted);

    //load the size 
    memcpy(&this->size, buffer+offset, sizeof(size));
    offset += sizeof(this->size);

    //this->fields.clear();

    cout<<this->size<<endl;
    // deserialzie each field
    while(offset < this->size){
        Field f(FieldType::TYPE_INT);
        f.deserialize_field(buffer + offset);
        offset += f.get_serialized_size();
        //f.print_field();
        fields.push_back(f);
    }

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

    cout<<"tuple size "<< tuple.get_tuple_size()<<endl;
    cout<<"is deleted "<< tuple.get_is_deleted()<<endl;

    tuple.print_tuple();
    tuple.mark_as_deleted();
    tuple.print_tuple();

    cout<<"testing tuple serialization "<<endl;

    char* buffer = new char[200];
    tuple.serialize_tuple(buffer);

    Tuple dummy_tuple({});
    dummy_tuple.deserialize_tuple(buffer);
    dummy_tuple.print_tuple();

}
*/