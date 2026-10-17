#pragma once


class RID{

private : 
    int page_id;
    int slot_num;

public:
    RID(int page_id=-1, int slot_num=-1):page_id(page_id), slot_num(slot_num){};

    int get_page_id();
    int get_slot_num();

    void serialize(char* data);
    void deserialize(char* data);
    int getSerializedSize();
    
    void print();  
};