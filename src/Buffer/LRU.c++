#include"Buffer/LRU.h"
#include"Storage/Page/Page.h"

Frame::Frame(int key, char*value){
    
    this->key = key;
    this->value = value;
    this->next = nullptr;
    this->prev = nullptr;
}

LRU::LRU(int capacity){
    
    this->capacity = capacity;
    //create dummy frames as head and tail
    this->head = new Frame();
    this->tail = new Frame();

    //then link them
    head->next = tail;
    tail->prev = head;
}

void LRU::put_frame(int key, char*value){
    
    //check if the page_id already exists :
    if(frames_map.find(key) != frames_map.end()){
        //fetch the frame then update it 
        Frame* curr_frame = frames_map[key];
        curr_frame->value = value;

        //then update it's position in the double linked list :
        
        //remove it first, then link 
        curr_frame->prev->next = curr_frame->next;
        curr_frame->next->prev = curr_frame->prev;

        //now move it to the first of the list 
        Frame* head_next = head->next;

        head_next->prev = curr_frame;
        curr_frame->next = head_next;

        head->next = curr_frame;
        curr_frame->prev = head;
        
        return;

    }
    //check if all frames are occupied: 
    //then evict the very last frame in the list (prev of tail) 
    if(frames_map.size() == capacity){
        
        //fetch the evicted page, then remove it 
        Frame* to_delete_frame  = tail->prev;
        frames_map.erase(to_delete_frame->key);

        //remove it from the list then link
        to_delete_frame->prev->next = tail;
        tail->prev = to_delete_frame->prev;

        delete to_delete_frame;

    }

    // now insert the new frame as the first (next to head)
    Frame* new_frame = new Frame(key, value);
    frames_map[key] = new_frame;

    //now move it to the first of the list 
    Frame* head_next = head->next;
    head_next->prev = new_frame;

    new_frame->next = head_next;
    head->next = new_frame;
    new_frame->prev = head;

    return;
}


char* LRU::get_frame(int key){
    
    //chekc if it's found in the list:
    if(frames_map.find(key) != frames_map.end()){

        Frame* curr_frame = frames_map[key];

        //remove it, then reinsert as first node
        curr_frame->prev->next = curr_frame->next;
        curr_frame->next->prev = curr_frame->prev;

        //now move it to the first of the list 
        Frame* head_next = head->next;
        head_next->prev = curr_frame;

        curr_frame->next = head_next;
        head->next = curr_frame;
        curr_frame->prev = head;

        return curr_frame->value;
    }

    return nullptr;
}


int LRU::evict_frame(){

    if(tail->prev == head){
        return -1;
    }
    
    Frame* evicted_frame = tail->prev;
    int frame_id = evicted_frame->key;
    frames_map.erase(frame_id);

    evicted_frame->prev->next = tail;
    tail->prev = evicted_frame->prev;

    delete evicted_frame;
    return frame_id;
}


void LRU::remove_frame(int key){

    Frame* evicted_frame = frames_map[key];

    int frame_id = evicted_frame->key;
    frames_map.erase(frame_id);

    evicted_frame->prev->next = evicted_frame->next;
    evicted_frame->next->prev = evicted_frame->prev;

    delete evicted_frame;
}


void LRU::print(){
    
    Frame* curr_frame = head->next;

    while(curr_frame != tail){
        cout<<curr_frame->key<<", ";
        curr_frame = curr_frame->next;
    }

    cout<<endl;
}

/*
int 
main(){
    cout<<"testing put and get"<<endl;
    {    
        LRU* lru = new LRU(3);

        Page* page1 = new Page(1);
        Page* page2 = new Page(2);
        Page* page3 = new Page(3);

        lru->put_frame(1, page1->getData());
        lru->put_frame(2, page1->getData());
        lru->put_frame(3, page1->getData());
    
        cout<<"put 1 then 2 then 3"<<endl;
        lru->print();

        char* page1_buffer =  lru->get_frame(1);
        assert(page1_buffer != nullptr);

        cout<<"get 1"<<endl;
        lru->print();

        char* page2_buffer =  lru->get_frame(2);
        assert(page2_buffer != nullptr);

        cout<<"get 2"<<endl;
        lru->print();

        char* page3_buffer =  lru->get_frame(3);
        assert(page3_buffer != nullptr);

        cout<<"get 3"<<endl;
        lru->print();

        char* page4_buffer =  lru->get_frame(4);
        assert(page4_buffer == nullptr);

        cout<<"get 4 (not found)"<<endl;
        lru->print();

    }
    cout<<"testing evict"<<endl;
    {    
        LRU* lru = new LRU(3);

        Page* page1 = new Page(1);
        Page* page2 = new Page(2);
        Page* page3 = new Page(3);

        lru->put_frame(1, page1->getData());
        lru->put_frame(2, page1->getData());
        lru->put_frame(3, page1->getData());
        
        cout<<"put 1, 2, 3"<<endl;
        lru->print();
        
        lru->put_frame(4, page1->getData());

        cout<<"put 4"<<endl;
        lru->print();

        lru->put_frame(5, page1->getData());

        cout<<"put 5"<<endl;
        lru->print();

        lru->put_frame(6, page1->getData());

        cout<<"put 6"<<endl;
        lru->print();
    }
}
*/