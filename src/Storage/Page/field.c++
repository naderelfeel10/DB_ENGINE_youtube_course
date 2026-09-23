#include"Storage/Page/field.h"


Field::Field(FieldType field_type, int value){
    this->field_type = FieldType(field_type);
    this->is_null = false;
    this->VALUE_INT = value;

    this->size = sizeof(value);
}

Field::Field(FieldType field_type, double value){
    this->field_type = FieldType(field_type);
    this->is_null = false;

    this->VALUE_DOUBLE = value;

    this->size = sizeof(value);
}

Field::Field(FieldType field_type, const char* value){

    this->field_type = FieldType(field_type);
    this->is_null = false;

    //value : "ELFEEL" + '\0'
    this->size = strlen(value);
    this->VALUE_STRING = new char[this->size + 1];
    //strcpy(dest, src)
    strcpy((char*)this->VALUE_STRING, value);
}

Field::Field(FieldType field_type, bool value){
    this->field_type = FieldType(field_type);
    this->is_null = false;
   
    this->VALUE_BOOL = value;

    this->size = sizeof(value);
}


Field::Field(FieldType field_type){
    this->field_type = FieldType(field_type);
    this->is_null = true;

    this->VALUE_BOOL = false;
    this->VALUE_INT = 0;
    this->VALUE_DOUBLE = 0.0;
    this->VALUE_STRING = "";

}

//getters 
FieldType Field::get_field_type(){
    return this->field_type;
}

int Field::get_field_size(){
    return this->size;
}

int Field::get_value_int(){
    return this->VALUE_INT;
}

double Field::get_value_double(){
    return this->VALUE_DOUBLE;
}

bool Field::get_value_bool(){
    return this->VALUE_BOOL;
}


string Field::get_value_string(){
    string str = string(this->VALUE_STRING);
    return str;
}

void Field::print_field(){

    // 10     |true   | 
    switch (this->field_type)
    {
    case FieldType::TYPE_INT:
        cout<<left << setw(15) << this->VALUE_INT;
        break;
    case FieldType::TYPE_DOUBLE:
        cout<<left << setw(15) << this->VALUE_DOUBLE;
        break;
    case FieldType::TYPE_BOOL:
        cout<<left << setw(15) << this->VALUE_BOOL;
        break;
    case FieldType::TYPE_STRING:
        cout<<left << setw(15) << this->VALUE_STRING;
        break;  
    default:
        break;
    }


    cout<<"|";

}

/*
    FieldType field_type;
    //size : 
    int size{};
    bool is_null;
    union {
        int  VALUE_INT;
        const char* VALUE_STRING;
        double VALUE_DOUBLE;
        bool VALUE_BOOL;
    };

*/
// [type,is_null, size, value ] 
void Field::serialize_field(char* buffer){
    
    int offset{1};

    //save is_null
    memcpy(buffer+offset , &this->is_null, sizeof(this->is_null));
    offset+= sizeof(this->is_null);

    //save size
    memcpy(buffer+offset , &this->size, sizeof(this->size));
    offset+= sizeof(this->size);

    //save the value :
    switch (this->field_type)
    {
    case FieldType::TYPE_INT:
        buffer[0] = 'I';
        memcpy(buffer+offset , &this->VALUE_INT, sizeof(this->VALUE_INT));
        offset+= sizeof(this->VALUE_INT);
        break;

    case FieldType::TYPE_BOOL:
        buffer[0] = 'B';
        memcpy(buffer+offset , &this->VALUE_BOOL, sizeof(this->VALUE_BOOL));
        offset+= sizeof(this->VALUE_BOOL);
        break;

    case FieldType::TYPE_DOUBLE:
        buffer[0] = 'D';
        memcpy(buffer+offset , &this->VALUE_DOUBLE, sizeof(this->VALUE_DOUBLE));
        offset+= sizeof(this->VALUE_DOUBLE);
        break;

    case FieldType::TYPE_STRING:
        buffer[0] = 'S';
        memcpy(buffer+offset , this->VALUE_STRING, size);
        buffer[offset+size] = '\0';
        offset+= size;
        break;

    default:

        break;
    }
}

void Field::deserialize_field(char* buffer){

    //fetch the type 
    char type = buffer[0];
    int offset{1};

    //load is_null
    memcpy(&this->is_null, buffer+offset, sizeof(this->is_null));
    offset += sizeof(this->is_null);

    //load size
    memcpy(&this->size, buffer+offset, sizeof(this->size));
    offset += sizeof(this->size);

    //load the values 
    //['I', 'B', 'S', 'D']
    switch (type)
    {
    case 'I':
        this->field_type = FieldType::TYPE_INT;
        memcpy(&this->VALUE_INT, buffer+offset, sizeof(this->VALUE_INT));
        offset += sizeof(this->VALUE_INT);
        break;

    case 'B':
        this->field_type = FieldType::TYPE_BOOL;
        memcpy(&this->VALUE_BOOL, buffer+offset, sizeof(this->VALUE_BOOL));
        offset += sizeof(this->VALUE_BOOL);
        break;

    case 'D':
        this->field_type = FieldType::TYPE_DOUBLE;
        memcpy(&this->VALUE_DOUBLE, buffer+offset, sizeof(this->VALUE_DOUBLE));
        offset += sizeof(this->VALUE_DOUBLE);
        break;

    case 'S':
        this->field_type = FieldType::TYPE_STRING;
        VALUE_STRING = new char[size + 1];
        
        memcpy((void*)VALUE_STRING, buffer+offset, size);
        ((char*)VALUE_STRING)[size] = '\0';
        offset+= size;
        break;
    default:
        break;
    }

}

int Field::get_serialized_size(){
    int res_size = 1 + sizeof(is_null) +  sizeof(size) + size;
    return res_size;

}

/*
int
main(){
    cout<<"testing field int"<<endl;
    {
        FieldType type = FieldType::TYPE_INT;
        int value = 9;

        Field f(type, value);
        cout<<"field size : " << f.get_field_size()<<endl;
        cout<<"field value : " << f.get_value_int()<<endl;

        assert(value == f.get_value_int());
        assert(sizeof(value) == f.get_field_size());
        
        f.print_field();
        cout<<endl;
        cout <<"serialization testing "<<endl;
        char* buffer = new char[100];   
        f.serialize_field(buffer);

        Field new_field(FieldType::TYPE_INT);
        new_field.deserialize_field(buffer);

        new_field.print_field();
        cout<<endl;
    }

    cout<<"testing field double"<<endl;
    {
        FieldType type = FieldType::TYPE_DOUBLE;
        double value = 9.1234;

        Field f(type, value);
        cout<<"field size : " << f.get_field_size()<<endl;
        cout<<"field value : " << f.get_value_double()<<endl;

        assert(value == f.get_value_double());
        assert(sizeof(value) == f.get_field_size());
        

        f.print_field();
        cout<<endl;
        cout <<"serialization testing "<<endl;
        char* buffer = new char[100];   
        f.serialize_field(buffer);

        Field new_field(FieldType::TYPE_INT);
        new_field.deserialize_field(buffer);

        new_field.print_field();
        cout<<endl;
    }

    cout<<"testing field string"<<endl;
    {
        FieldType type = FieldType::TYPE_STRING;
        const char* value = "ELFEEL";

        Field f(type, value);
        cout<<"field size : " << f.get_field_size()<<endl;
        cout<<"field value : " << f.get_value_string()<<endl;

        assert(value == f.get_value_string());
        assert(strlen(value) == f.get_field_size());
        
        f.print_field();
        cout<<endl;
        cout <<"serialization testing "<<endl;
        char* buffer = new char[100];   
        f.serialize_field(buffer);

        Field new_field(FieldType::TYPE_INT);
        new_field.deserialize_field(buffer);

        new_field.print_field();
        cout<<endl;
    }
}
*/