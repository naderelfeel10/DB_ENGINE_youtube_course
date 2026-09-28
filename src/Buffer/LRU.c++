#include"Buffer/LRU.h"
#include<iostream>

Node::Node(int key, char* value){

    this->key = key;
    this->value = value;
    this->next = nullptr;
    this->prev = nullptr;
}


LRU::LRU(int capacity){
    
    this->capacity = capacity;

    this->head = new Node();
    this->tail = new Node();

    head->next = tail;
    tail->prev = head;

}


void LRU::put_node(int key, char* value){

    //first, check if we have this node or not
    //if found, then update it's value, then update it's location 
    
    if(this->nodes_map.find(key) != nodes_map.end()){
        
        //update old value with new value 
        Node* curr_node = nodes_map[key];
        curr_node->value = value;

        //udpate the position of the node
        // make it next to the head

        //first, remove it : 
        curr_node->prev->next = curr_node->next;
        curr_node->next->prev = curr_node->prev;

        // move it to the begining of the list :
        Node* head_next = head->next;

        head_next->prev = curr_node;
        curr_node->next = head_next;

        head->next = curr_node;
        curr_node->prev = head;

        return ;

    }
    //if not found, then we have to insert it: 
        // if we have empty room for it, then just insert it as next to the head
        // if not, we have to evict one (the prev of the Tail)

    //if no space : 
    if(nodes_map.size() == capacity){

        //fetch the node to evict : 
        Node* evicted_node = tail->prev;
        nodes_map.erase(evicted_node->key);

        //then update the pointers 
        evicted_node->prev->next = tail;
        tail->prev = evicted_node->prev;

        delete evicted_node;
    }

    // now we have a room for new Node : 
    // create the new node, then insert it as the first node in the list : 
    Node* new_node = new Node(key, value);
    nodes_map[key] = new_node;

    // then update the pointers :

    Node* head_next = head->next;

    head_next->prev = new_node;
    new_node->next = head_next;

    head->next = new_node;
    new_node->prev = head;

    return;
}   


char* LRU::get_frame(int key){

    //check for the key in nodes_map
    if(nodes_map.find(key) != nodes_map.end()){

        Node* curr_node = nodes_map[key];

        //udpate the position of the node
        // make it next to the head

        //first, remove it : 
        curr_node->prev->next = curr_node->next;
        curr_node->next->prev = curr_node->prev;

        // move it to the begining of the list :
        Node* head_next = head->next;

        head_next->prev = curr_node;
        curr_node->next = head_next;

        head->next = curr_node;
        curr_node->prev = head;
        
        //return the page_data
        return curr_node->value;
        
    }
    return nullptr;
}
int LRU::evict_frame(){

    Node* evicted_node = tail->prev;

    int node_id = evicted_node->key;

    //remove it from the list 
    nodes_map.erase(node_id);

    evicted_node->prev->next = tail;
    tail->prev = evicted_node->prev;

    delete evicted_node;

    return node_id;
}

void LRU::remove_frame(int key){

    Node* evicted_node = nodes_map[key];

    int node_id = evicted_node->key;

    //remove it from the list 
    nodes_map.erase(node_id);

    //update the pointers :
    evicted_node->prev->next = evicted_node->next;
    evicted_node->next->prev = evicted_node->prev;

    delete evicted_node;
}

void LRU::print(){

    Node* curr_node = head->next;

    while(curr_node != tail){
        cout<<curr_node->key<<", ";
        curr_node = curr_node->next;
    } 

    cout<<endl;
}   

/*
int 
main(){

    cout<<"testing LRU"<<endl;

    {
        LRU* lru = new LRU(3);

        Page* page1 = new Page(1);
        Page* page2 = new Page(2);
        Page* page3 = new Page(3);

        lru->put_node(1, page1->getData());
        lru->put_node(2, page2->getData());
        lru->put_node(3, page3->getData());

        cout<<"3, 2, 1"<<endl;
        char* page_3_buffer = lru->get_frame(3);
        assert(page_3_buffer != nullptr);
        lru->print();

        cout<<"inserting 4"<<endl;
        lru->put_node(4, page3->getData());

        char* page_1_buffer = lru->get_frame(1);
        assert(page_1_buffer == nullptr);
        cout<<"4, 3, 2"<<endl;
        lru->print();

        cout<<"access page2"<<endl;
        char* page_2_buffer = lru->get_frame(2);
        assert(page_2_buffer != nullptr);
        cout<<"2, 4, 3"<<endl;
        lru->print();

    }

    cout<<"tesing evict frame"<<endl;
    {
        LRU* lru = new LRU(3);

        Page* page1 = new Page(1);
        Page* page2 = new Page(2);
        Page* page3 = new Page(3);

        lru->put_node(1, page1->getData());
        lru->put_node(2, page2->getData());
        lru->put_node(3, page3->getData());

        cout<<"3, 2, 1"<<endl;

        char* page_3_buffer = lru->get_frame(3);
        assert(page_3_buffer != nullptr);
        lru->print();

        lru->evict_frame();
        cout<<"3, 2"<<endl;
        lru->print();

        char* page_1_buffer = lru->get_frame(1);
        assert(page_1_buffer == nullptr);

    }

    cout<<"tesing remove frame"<<endl;
    {
        LRU* lru = new LRU(3);

        Page* page1 = new Page(1);
        Page* page2 = new Page(2);
        Page* page3 = new Page(3);

        lru->put_node(1, page1->getData());
        lru->put_node(2, page2->getData());
        lru->put_node(3, page3->getData());

        cout<<"3, 2, 1"<<endl;

        char* page_3_buffer = lru->get_frame(3);
        assert(page_3_buffer != nullptr);
        lru->print();

        lru->remove_frame(2);
        cout<<"3, 1"<<endl;
        lru->print();

        char* page_2_buffer = lru->get_frame(2);
        assert(page_2_buffer == nullptr);

    }
}
*/