#pragma once 


#include<unordered_map>
#include<iostream>
// frame is the node of the LRU : 
// consist of : page_id, page_data, (prev, next) pointers
using namespace std;

class Frame{
public:

    int key;
    char* value;
    Frame* next;
    Frame* prev;

    Frame(int key=-1, char* value=nullptr);
};
//lru class who has the functions to put, evict or get frames 
// consists of : head and tail nodes and a capacity of the buffer pool manager
class LRU{
private:
    //dummy head and tail 
    Frame* head;
    Frame* tail;
    int capacity;
    unordered_map<int, Frame*>frames_map;

public:
    LRU(int capacity);
    void put_frame(int key, char*value);
    char* get_frame(int key);

    int evict_frame();
    void remove_frame(int key);
    
    void print();
};