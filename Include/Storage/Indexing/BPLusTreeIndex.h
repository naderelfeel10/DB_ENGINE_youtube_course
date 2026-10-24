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


    BPlusNode(bool leaf):is_leaf(leaf), next(nullptr), page_id(-1), next_page_id(-1){}

};


class BPLusTree{

//private:
public:
    
    //root node 
    BPlusNode* root;
    //min degree
    int t;
    BufferPoolManager* BPM;

    void splitChild(BPlusNode* parent, int index ,BPlusNode* child);
    void insertNonFull(BPlusNode* root, Field key, RID value);

    BPlusNode* loadNode(int page_id);
    void loadChildren(BPlusNode* node);
    BPlusNode* getNextLeaf(BPlusNode* node);


    BPLusTree(BufferPoolManager*BPM, int t, int root_page_id):BPM(BPM), t(t), root(nullptr){
        
        if(root_page_id != -1){
            root = loadNode(root_page_id);
        }
    };
    void printTree();
    void printLeaves();

    //functions:
    void insert(Field key, RID value);
    vector<RID> search(Field& key);    
    vector<RID> searchRange(Field& lower_key, Field& upper_key);
};