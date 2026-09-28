#pragma once 

#include"Tuple.h"
#define PAGE_SIZE 4096

//contains meta data about the page
struct PageHeader{
    int page_id;
    int next_page_id{-1};
    int num_of_tuples;
    int num_of_deleted_tuples;
    int offset_pointer;
};

struct Slot{
    int offset;
    int size;
    bool is_deleted{false};
};


class Page{

private:
    char data[PAGE_SIZE];

public:
    Page(int page_id);

    //getter 
    int get_page_id();

    //crud : 
    int insert_tuple(Tuple tuple);
    bool get_tuple(int slot_num, Tuple& tuple);
    bool delete_tuple(int slot_num);
    int update_tuple(int slot_num, Tuple new_tuple);

    char* getData(){
        return this->data;
    }
};