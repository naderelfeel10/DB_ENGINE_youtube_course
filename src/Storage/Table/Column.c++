#include"Storage/Table/Column.h"


//setters and getters : 
string Column::get_col_name(){
    return this->col_name;
}

void Column::set_col_name(string col_name){
    this->col_name = col_name;
}

FieldType Column::get_field_type(){
    return this->field_type;
}

void Column::set_field_type_and_size(FieldType field_type, int max_size){
    
    this->field_type = field_type;
    this->max_size = max_size;
}

int Column::get_col_max_size(){
    return this->max_size;
}

bool Column::operator==(const Column& other) const{

    //compare field_type, name, and size
    return field_type == other.field_type && col_name == other.col_name && max_size == other.max_size;
}

bool Column::operator!=(const Column& other) const {
    return !(*this == other);
}