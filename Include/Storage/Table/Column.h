#pragma once


#include<Storage/Page/field.h>

class Column{

private:
    //Field* field;
    FieldType field_type;
    string col_name;
    int max_size;

public:
    
    Column(FieldType field_type, string col_name, int max_size):field_type(field_type)
                                                                ,col_name(col_name)
                                                                ,max_size(max_size){};

    Column():col_name("UNKNOWN"),max_size(-1){};

    //setters and getters : 
    string get_col_name();
    void set_col_name(string col_name);

    FieldType get_field_type();
    void set_field_type_and_size(FieldType field_type, int max_size);

    int get_col_max_size();
    
    bool operator==(const Column& other) const;
    bool operator!=(const Column& other) const;


}; 