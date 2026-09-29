#include"Storage/Table/RID.h"

#include<iostream>


int RID::get_page_id(){
    return this->page_id;
}

int RID::get_slot_num(){
    return this->slot_num;
}

void RID::print(){
    std::cout<<"("<<page_id<<", "<<slot_num<<")"<<std::endl;
}