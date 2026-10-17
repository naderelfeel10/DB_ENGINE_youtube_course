#pragma once 


#include<vector>
#include <string>
#include"Storage/Page/field.h"
#include"Storage/Table/RID.h"
#include<iostream>
#include<algorithm>
#include"Storage/Page/Page.h"
#include"Buffer/BufferPoolManager.h"


using namespace std;

struct BPlusNode;

struct Entry{

    Field key;
    RID value;

    int serialize(char* data);
    int deserialize(char* data);

    Entry(): key(Field(FieldType::TYPE_INT)), value(RID(-1,-1)){};
    Entry(Field key, RID rid):key(key), value(rid){};

};
struct BPlusNode{

    bool is_leaf;
    vector<Entry>entries; // for the leaf nodes
    vector<Field>keys; // for the internal nodes
    vector<BPlusNode*>children;
    BPlusNode* next;
    
    //for savign nodes to the disk
    vector<int>childern_pages_ids;
    int page_id;
    int next_page_id;

    void serialize(char* data);
    void deserialize(char* data);


    BPlusNode(bool is_leaf):is_leaf(is_leaf){};
};


class BPLusTree{

private:
    //root node 
    BPlusNode* root;
    //min degree
    int t;
    BufferPoolManager* BPM;

    void splitChild(BPlusNode* parent, int index ,BPlusNode* child);
    void insertNonFull(BPlusNode* root, Field key, RID value);

public:
    BPLusTree(BufferPoolManager*BPM, int t):BPM(BPM), t(t), root(nullptr){};

    //functions:
    void insert(Field key, RID value);
    void printTree();
    void printLeaves();
    //void search(Field key);

};