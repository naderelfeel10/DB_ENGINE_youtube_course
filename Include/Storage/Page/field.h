#pragma once 

#include"cstring"
#include <string>
#include"cassert"
#include <iomanip>
#include"iostream"

using namespace std;
//field types
enum class FieldType {TYPE_INT, TYPE_DOUBLE, TYPE_BOOL, TYPE_STRING};


class Field{

private:
    //field_type: int, double, bool, string] 
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

public:
    //constructors
    Field(FieldType field_type, int value);
    Field(FieldType field_type, double value);
    Field(FieldType field_type, const char* value);
    Field(FieldType field_type, bool value);

    Field(FieldType field_type);

    //getters :
    FieldType get_field_type();
    int get_field_size();

    int get_value_int();
    double get_value_double();
    bool get_value_bool();
    string get_value_string();

    void print_field();
    void serialize_field(char* buffer);
    void deserialize_field(char* buffer);
    int get_serialized_size();
};