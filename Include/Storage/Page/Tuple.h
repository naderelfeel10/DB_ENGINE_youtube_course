#pragma once 


#include"field.h"

#include<vector>

class Tuple{

private:
    // tuple is just a vector of fields
    vector<Field>fields;

    //meta data : 
    bool is_deleted;
    int size;

public:

    Tuple(vector<Field>fields);


    //gettes :
    bool get_is_deleted();
    int get_tuple_size(); 
    vector<Field> get_tuple_fields();

    void print_tuple();

    void mark_as_deleted();
    
    void serialize_tuple(char* buffer);
    void deserialize_tuple(char* buffer);
};