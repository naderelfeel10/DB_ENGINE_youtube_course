#include"Storage/Table/RID.h"
#include<iostream>
#include<cstring>



int RID::get_page_id(){
    return this->page_id;
}

int RID::get_slot_num(){
    return this->slot_num;
}
void RID::serialize(char* data){

    int offset{0};
    // serialize page_id
    memcpy(data+offset, &page_id, sizeof(int));
    offset+=sizeof(int);

    // serialize slot_num
    memcpy(data+offset, &slot_num, sizeof(int));
    offset+=sizeof(int);
}

void RID::deserialize(char *data){
    int offset{0};

    // deserialize page_id
    memcpy(&this->page_id,data+offset, sizeof(int));
    offset+=sizeof(int);

    // deserialize slot_num
    memcpy(&this->slot_num,data+offset, sizeof(int));
    offset+=sizeof(int);

}

int RID::getSerializedSize(){
    return sizeof(page_id) + sizeof(slot_num);
}

void RID::print(){
    std::cout<<"("<<page_id<<", "<<slot_num<<")"<<std::endl;
}