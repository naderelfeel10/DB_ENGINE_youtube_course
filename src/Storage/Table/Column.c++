#include"Storage/Table/Column.h"

int Column::serialize_col(char* buffer){
    int offset{1};

    //save field_type
    switch (this->field_type){

    case FieldType::TYPE_INT:
        buffer[0] = 'I';
        break;

    case FieldType::TYPE_BOOL:
        buffer[0] = 'B';
        break;

    case FieldType::TYPE_DOUBLE:
        buffer[0] = 'D';
        break;

    case FieldType::TYPE_STRING:
        buffer[0] = 'S';
        break;
    }

    //save col_name size : 
    int col_name_size = col_name.size();
    memcpy(buffer+offset, &col_name_size, sizeof(col_name_size));
    offset+=sizeof(col_name_size);

    //save col_name
    memcpy(buffer+offset , col_name.data(), col_name_size);
    offset+= col_name_size;

    return offset;
}

int Column::deserialize_col(const char* buffer){
    
    int offset{1};

    // read field_type
    switch (buffer[0]){
        case 'I':
            this->field_type = FieldType::TYPE_INT;
            break;

        case 'B':
            this->field_type = FieldType::TYPE_BOOL;
            break;

        case 'D':
            this->field_type = FieldType::TYPE_DOUBLE;
            break;

        case 'S':
            this->field_type = FieldType::TYPE_STRING;
            break;

        default:
            throw std::runtime_error("invalid column field type");
    }

    //read col_name size
    int col_name_size;

    memcpy(&col_name_size, buffer + offset, sizeof(col_name_size));
    offset += sizeof(col_name_size);

    //read col_name
    this->col_name.assign(buffer + offset, col_name_size);
    offset += col_name_size;

    return offset;
}

void Column::print(){
    cout<<"------"<<endl;
    cout<<col_name<<endl;
    cout<<"------"<<endl;
}