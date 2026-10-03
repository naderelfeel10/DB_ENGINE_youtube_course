#pragma once 


#include"Storage/Page/field.h"

class Column{
private: 
    FieldType field_type;
    string col_name;
    int mx_size;
public:
    Column(FieldType field_type, string col_name, int mx_size):field_type(field_type), col_name(col_name)
                                                                ,mx_size(mx_size){};
   
    Column():col_name("UNKOWN"), mx_size(-1){};
    
    FieldType get_field_type(){
        return this->field_type;
    }

    int serialize_col(char* buffer);
    int deserialize_col(const char* buffer);
    void print();
};