#pragma once 

#include"unordered_map"
#include"Storage/Page/Page.h"
using namespace std;

class Node{

public:
    //key : page_id 
    int key;
    //page_data
    char* value;

    //pointers
    Node* next;
    Node* prev;
    Node(int key=-1, char* value=nullptr);
};


class LRU{

private:

    Node* head;
    Node* tail;
    //how many pages it can hold
    int capacity;
    unordered_map<int, Node*> nodes_map;
public:

    LRU(int capacity);
    void put_node(int key, char* value);
    char* get_frame(int key);
    int evict_frame();
    void remove_frame(int key);


    void print();

};