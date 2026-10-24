#include"Storage/Indexing/BPLusTreeIndex.h"




void BPLusTree::insert(Field key, RID value){

    //if it's the first entry in the tree, then create a new root;
    if(root == nullptr){

        //create new node :
        root = new BPlusNode(true);

        //create page for this new node
        root->page_id = BPM->newPage();
        cout<<"root page_id : "<<root->page_id<<endl;

        Entry entry = Entry(key, value);    
        root->entries.push_back(entry);
        root->keys.push_back(key);

        //save this node on disk
        char* page_buffer = BPM->fetchPage(root->page_id);
        root->serialize(page_buffer);
        BPM->mark_as_dirty(root->page_id);

    }else{

        //check if the node is full or not 
        int root_size = root->is_leaf? root->entries.size() : root->keys.size();
        if(root_size == 2*t-1){
            
            BPlusNode* new_root = new BPlusNode(false);

            //create page for this new node
            new_root->page_id = BPM->newPage();
            
            new_root->children.push_back(root);
            new_root->childern_pages_ids.push_back(root->page_id);

            splitChild(new_root, 0, root);
            root = new_root;

            //save this node on disk
            char* page_buffer = BPM->fetchPage(new_root->page_id);
            new_root->serialize(page_buffer);
            BPM->mark_as_dirty(new_root->page_id);
        }
        //insert the new entry : 
        insertNonFull(root, key, value);

    }
}


void BPLusTree::splitChild(BPlusNode* parent, int index, BPlusNode* child){
    
    BPlusNode* new_child = new BPlusNode(child->is_leaf);
    new_child->page_id = BPM->newPage();

    if(child->is_leaf){

        new_child->entries.assign(
            child->entries.begin() + (t - 1),
            child->entries.end()
        );

        child->entries.erase(
            child->entries.begin() + (t - 1),
            child->entries.end()
        );

        child->keys.clear();
        for (const auto& entry : child->entries) {
            child->keys.push_back(entry.key);
        }

        new_child->keys.clear();
        for (const auto& entry : new_child->entries) {
            new_child->keys.push_back(entry.key);
        }

        parent->keys.insert(
            parent->keys.begin() + index,
            new_child->entries.front().key
        );

        new_child->next = child->next;
        new_child->next_page_id = child->next_page_id;
        
        child->next = new_child;
        child->next_page_id = new_child->page_id;
    }
    else
    {
        Field middle_key = child->keys[t - 1];

        parent->keys.insert(
            parent->keys.begin() + index,
            middle_key
        );

        new_child->keys.assign(
            child->keys.begin() + t,
            child->keys.end()
        );

        child->keys.erase(
            child->keys.begin() + (t - 1),
            child->keys.end()
        );

        new_child->children.assign(
            child->children.begin() + t,
            child->children.end()
        );

        child->children.erase(
            child->children.begin() + t,
            child->children.end()
        );
    }

    parent->children.insert(parent->children.begin() + index + 1, new_child);

    parent->childern_pages_ids.insert(parent->childern_pages_ids.begin() + index + 1,
         new_child->page_id);


    char* parent_buf = BPM->fetchPage(parent->page_id);
    parent->serialize(parent_buf);
    BPM->mark_as_dirty(parent->page_id);

    char* child_buf = BPM->fetchPage(child->page_id);
    child->serialize(child_buf);
    BPM->mark_as_dirty(child->page_id);

    char* new_child_buf = BPM->fetchPage(new_child->page_id);
    new_child->serialize(new_child_buf);
    BPM->mark_as_dirty(new_child->page_id);
}


void BPLusTree::insertNonFull(BPlusNode* root, Field key, RID value){

    if(root->is_leaf == true){

        //insert into the correct position : 
        auto it = lower_bound(root->entries.begin(), root->entries.end(), key, 
        [](const Entry& e, const Field&k){return e.key < k;});

        root->entries.insert(it, Entry(key, value));

        root->keys.clear();
        for(const auto&e:root->entries){
            root->keys.push_back(e.key);
        }

    }else{
        int i = root->keys.size()-1;
        while(i>=0 && key < root->keys[i]){
            i--;
        }
        i++;

        
        root->children[i] = loadNode(root->childern_pages_ids[i]);

        BPlusNode* child = root->children[i];
        int size = child->is_leaf ? child->entries.size() : child->keys.size();

        if(size == 2*t-1){  
            splitChild(root, i, child);
            if(key > root->keys[i]){
                i++;
            }
            child = root->children[i];
        }

        //insert it again  : 
        insertNonFull(child, key, value);
        
    }

    char* page_buffer = BPM->fetchPage(root->page_id);
    root->serialize(page_buffer);
    BPM->mark_as_dirty(root->page_id);
}

void BPLusTree::printTree(){

    if (root == nullptr)
    {
        cout << "Tree is empty\n";
        return;
    }

    vector<BPlusNode*> current_level;
    current_level.push_back(root);

    int level = 0;

    while (!current_level.empty())
    {
        cout << "Level " << level << ": ";

        vector<BPlusNode*> next_level;

        for (BPlusNode* node : current_level)
        {
            if (node == nullptr) {
                continue;
            }

            cout << "[";

            for (size_t i = 0;
                 i < node->keys.size();
                 ++i)
            {
                node->keys[i].print_field();

                if (i + 1 < node->keys.size()) {
                    cout << " | ";
                }
            }

            cout << "] ";

            if (!node->is_leaf)
            {
                loadChildren(node);

                for (BPlusNode* child : node->children)
                {
                    if (child != nullptr) {
                        next_level.push_back(child);
                    }
                }
            }
        }

        cout << "\n";

        current_level = next_level;
        ++level;
    }
}


BPlusNode* BPLusTree::getNextLeaf(BPlusNode* node)
{
    if (node == nullptr) {
        return nullptr;
    }

    if (node->next == nullptr &&
        node->next_page_id != -1)
    {
        node->next = loadNode(node->next_page_id);
    }

    return node->next;
}
void BPLusTree::printLeaves()
{
    if (root == nullptr) {
        cout << "Leaf chain is empty\n";
        return;
    }

    BPlusNode* node = root;

    while (!node->is_leaf)
    {
        node = node->children.front();
    }

    cout << "Leaf chain:\n";

    int steps = 0;

    while (node != nullptr && steps < 20)
    {
        cout << "[page_id=" << node->page_id
             << ", next_page_id=" << node->next_page_id
             << "] ";

        for (auto& entry : node->entries) {
            entry.key.print_field();
        }

        cout << "\n";

        if (node->next_page_id == -1) {
            break;
        }

        if (node->next_page_id == node->page_id) {
            cout << "ERROR: leaf points to itself\n";
            break;
        }

        node = loadNode(node->next_page_id);
        ++steps;
    }

    if (steps == 20) {
        cout << "ERROR: possible cycle in leaf chain\n";
    }
}



//save entry to disk
int Entry::serialize(char* data){
    int offset{0};

    key.serialize_field(data+offset);
    offset+=key.get_serialized_size();

    value.serialize(data+offset);
    offset+=value.getSerializedSize();

    return offset;
}


//load entry from disk
int Entry::deserialize(char* data){

    int offset{0};

    this->key.deserialize_field(data+offset);
    offset+=this->key.get_serialized_size();

    this->value.deserialize(data+offset);
    offset+=this->value.getSerializedSize();
    
    return offset;
}


//saving each node in one full page
void BPlusNode::serialize(char* data){

    int offset = sizeof(PageHeader);

    //save is_leaf: 
    memcpy(data + offset, &is_leaf, sizeof(is_leaf));
    offset += sizeof(is_leaf);

    //save next page id
    memcpy(data + offset, &next_page_id, sizeof(next_page_id));
    offset += sizeof(next_page_id);

    //save children pages_ids:
    //save vector size first : 
    int childern_size = childern_pages_ids.size();
    
    memcpy(data + offset, &childern_size, sizeof(childern_size));
    offset += sizeof(childern_size);   

    for(auto& id: childern_pages_ids){
        memcpy(data + offset, &id, sizeof(id));
        offset += sizeof(id);
    }

    //save entries 
    //save entries size : 
    int entries_size = entries.size();
    
    memcpy(data + offset, &entries_size, sizeof(entries_size));
    offset += sizeof(entries_size);  

    for(auto& entry: entries){
        int size =  entry.serialize(data + offset);
        offset += size;
    }

    //save keys 
    //save keys size : 
    int keys_size = keys.size();
    
    memcpy(data + offset, &keys_size, sizeof(keys_size));
    offset += sizeof(keys_size);  

    for(auto& key: keys){
        key.serialize_field(data + offset);
        offset += key.get_serialized_size();
    }

}


void BPlusNode::deserialize(char* data){

    int offset = sizeof(PageHeader);

    //load is_leaf
    memcpy(&is_leaf, data + offset, sizeof(is_leaf));
    offset += sizeof(is_leaf);
    
    // load next page ID
    memcpy(&next_page_id, data + offset, sizeof(next_page_id));
    offset += sizeof(next_page_id);

    // load children page ids size
    int children_size = 0;

    memcpy(&children_size,data + offset,sizeof(children_size));
    offset += sizeof(children_size);

    //load children ids
    childern_pages_ids.clear();
    childern_pages_ids.reserve(children_size);

    for(int i{};i<children_size; ++i){

        int page_id = -1;
        memcpy(&page_id, data + offset, sizeof(page_id));
        offset += sizeof(page_id);

        childern_pages_ids.push_back(page_id);
    }

    children.clear();
    children.resize(children_size, nullptr);

    // load entries count
    int entries_size = 0;

    memcpy(&entries_size, data + offset, sizeof(entries_size));
    offset += sizeof(entries_size);

    entries.clear();
    entries.reserve(entries_size);

    //load entries
    for(int i = 0; i <entries_size; ++i){
        Entry entry;

        int serialized_size = entry.deserialize(data + offset);

        offset += serialized_size;
        entries.push_back(entry);
    }

    // load keys sizw
    int keys_size = 0;

    memcpy(&keys_size, data + offset, sizeof(keys_size));
    offset += sizeof(keys_size);

    keys.clear();
    keys.reserve(keys_size);

    //load keys
    for(int i = 0; i<keys_size; ++i){
        Field key(FieldType::TYPE_INT);

        key.deserialize_field(data + offset);

        offset += key.get_serialized_size();
        keys.push_back(key);
    }

    next = nullptr;
}

BPlusNode* BPLusTree::loadNode(int page_id){

    char* page_buffer = BPM->fetchPage(page_id);

    if (page_buffer == nullptr) {
        return nullptr;
    }

    BPlusNode* node = new BPlusNode(true);

    node->page_id = page_id;
    node->deserialize(page_buffer);

    return node;
}

void BPLusTree::loadChildren(BPlusNode* node){

    if(node == nullptr || node->is_leaf){
        return;
    }

    if(node->children.size() != node->childern_pages_ids.size()){
        node->children.resize(node->childern_pages_ids.size(), nullptr);
    }

    for (size_t i = 0; i < node->childern_pages_ids.size(); ++i){
        if (node->children[i] == nullptr){
            node->children[i] = loadNode(node->childern_pages_ids[i]);
        }
    }
}



int 
main(){

    /*{
        const string file_name = "B+Tree.bin";
        DiskManager* dm = new DiskManager(file_name);
        BufferPoolManager* BPM = new BufferPoolManager(dm);

        BPLusTree tree(BPM,2, -1);

        tree.insert(Field(FieldType::TYPE_INT, 19), RID(1, 0));
        tree.insert(Field(FieldType::TYPE_INT, 5),  RID(1, 1));
        tree.insert(Field(FieldType::TYPE_INT, 30), RID(1, 2));
        tree.insert(Field(FieldType::TYPE_INT, 10), RID(1, 3));
        tree.insert(Field(FieldType::TYPE_INT, 25), RID(1, 4));
        tree.insert(Field(FieldType::TYPE_INT, 40), RID(1, 5));

        tree.printTree();
        cout<<"---------------------"<<endl;
        tree.printLeaves();
        
        BPM->~BufferPoolManager();

    }*/
    {
        const string file_name = "B+Tree.bin";
        DiskManager* dm = new DiskManager(file_name);
        BufferPoolManager* BPM = new BufferPoolManager(dm);


        BPLusTree tree(BPM,2, 2);
    
    
        tree.printTree();
        cout<<"---------------------"<<endl;
        tree.printLeaves();
    
    }
    
}   