#include"Storage/Indexing/BPlusTreeIndex.h"



void BPlusTree::insert(Field key, RID value){

    //the is empty : 
    if(root == nullptr){

        //create a new node
        root = new Node(true);
        
        //create it's page as well 
        root->curr_page_id = BPM->newPage();
        
        //insett the entry into entries 
        Entry entry =  Entry(key, value);
        root->entries.push_back(entry);

        root->keys.push_back(key);
    }else{
        //check if the root is full : 
        int root_size = root->is_leaf ? root->entries.size() : root->keys.size();
        //if full, create new node, and split the old root and update curr_root
        if(root_size == 2*t-1){
            //create new node
            Node* new_root = new Node(false);
            new_root->curr_page_id = BPM->newPage();

            //assign old root as child of new root, then split the old root 
            new_root->children.push_back(root);
            new_root->children_pages_ids.push_back(root->curr_page_id);

            splitChild(new_root, 0, root);
            root = new_root;
        }
        //insert it normally 
        insertNonFull(root, key, value);
    }

}


void BPlusTree::splitChild(Node* parent, int index, Node* child){

    //create a new child :
    bool is_leaf = child->is_leaf;
    Node* new_child = new Node(is_leaf);
    new_child->curr_page_id = BPM->newPage();

    //check weather the node is leaf or not : 
    if(child->is_leaf){
        //split the entries, keys between child, and new child
        new_child->entries.assign(child->entries.begin() + t -1, child->entries.end());
        child->entries.resize(t-1);

        child->keys.clear();
        for(const auto&e:child->entries){
            child->keys.push_back(e.key);
        }

        new_child->keys.clear();
        for(const auto&e:new_child->entries){
            new_child->keys.push_back(e.key);
        }

        // insert the first key of the right child to the parent:
        parent->keys.insert(parent->keys.begin()+index, new_child->entries[0].key);

        //update pointers 
        new_child->next_page_id = child->next_page_id;
        new_child->next = child->next;

        child->next_page_id = new_child->curr_page_id;
        new_child->next = child->next;
        child->next_page_id = new_child->curr_page_id;
        child->next = new_child;
    
    }else{

        //if it's not leaf node:
        //insert the t-th key into the parent 
        parent->keys.insert(parent->keys.begin()+index, child->keys[t-1]);

        //split the entries, keys between child, and new child
        new_child->keys.assign(child->keys.begin() + t, child->keys.end());
        child->keys.resize(t - 1);
        
        new_child->children_pages_ids.assign(child->children_pages_ids.begin() + t, child->children_pages_ids.end());
        child->children_pages_ids.resize(t);

        new_child->children.assign(child->children.begin() + t, child->children.end());
        child->children.resize(t);        

    }
    //finally insert the new child to parent childs: 
    parent->children.insert(parent->children.begin()+index+1, new_child);

    parent->children_pages_ids.insert(parent->children_pages_ids.begin()+index+1, new_child->curr_page_id);

}

void BPlusTree::insertNonFull(Node* node, Field key, RID value){

    if(node->is_leaf){
        //insert the key in the correct position
        auto it = lower_bound(node->entries.begin(), node->entries.end(), key, 
            [](Entry& e, Field& k){return e.key < k;});

        node->entries.insert(it, Entry(key, value));

        node->keys.clear();
        for (const auto& entry : node->entries) {
            node->keys.push_back(entry.key);
        }    

        char* page_buffer = BPM->fetchPage(node->curr_page_id);
        node->serialize(page_buffer);
        BPM->mark_as_dirty(node->curr_page_id);
    
    }
    //if the node is internal node 
    else {
        
        //find the index of the key to insert : 
        int i= node->keys.size()-1;
        while(i>=0 && key < node->keys[i]){
            i--;
        }
        i++;

        if (node->children.size() <= i) {
            node->children.resize(node->children_pages_ids.size(), nullptr);
        }
        //now load the child in order to insert data into
        if (node->children[i] == nullptr) {
            node->children[i] = loadNode(node->children_pages_ids[i]);
        } 

        Node* child = node->children[i];
        size_t child_size = child->is_leaf ? child->entries.size() : child->keys.size();

        //if the internal node is alos full, then split it
        //then get the new child
        if(child_size == 2 * t - 1){ 

            splitChild(node, i, child);

            if (key > node->keys[i]) {
                i++;
            }
            child = node->children[i];
        }

        //insertt again
        insertNonFull(child, key, value);
        
        char* page_buffer = BPM->fetchPage(node->curr_page_id);
        node->serialize(page_buffer);
        BPM->mark_as_dirty(node->curr_page_id);

    }
}

bool BPlusTree::search(Field key){

}