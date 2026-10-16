#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include"Storage/Page/field.h"
#include"Storage/Table/RID.h"
#include"Buffer/BufferPoolManager.h"

using namespace std;

struct Node;


//entry represents the actual data we are storing in the leaf node : 
// (key, value) pair

struct Entry{

    Field key;
    RID value; // that's for leaf nodes only, inner nodes does not have values
    Node* next; // that's for inner nodes, a pointer to the vertical node 

    Entry();
    Entry(Field key, RID rid):key(key), value(value){};
};


// each node has vector of entries used in the leaf node 
// vector of keys (fields)
// vector of nodes (children)
// a pointer to the next node
struct Node{
    
    bool is_leaf;
    vector<Entry>entries;
    vector<Field>keys;
    vector<Node*>children;

    vector<int> children_pages_ids;
    int curr_page_id;
    int next_page_id;
    Node* next; // that's the sibling node, points horizontaly to the next node 

    Node(bool is_leaf);
};


// B+ Tree : 
/*
    constructs of : 
            root node(this will carry the list of nodes as tree)

    min degree of each node

    functions to insert, delete and search through the tree
*/


class BPlusTree{

private:
    Node* root;
    // min degree 
    int t;
    BufferPoolManager* BPM;
    void splitChild(Node* parent, int index, Node* child);
    void insertNonFull(Node* root, Field key, RID value);
    
public:
    BPlusTree(int t): t(t), root(nullptr), BPM(BPM){};
    
    void insert(Field key, RID value);
    bool search(Field key);
}; 



