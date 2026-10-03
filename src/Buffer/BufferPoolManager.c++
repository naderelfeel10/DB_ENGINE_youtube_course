#include"Buffer/BufferPoolManager.h"


BufferPoolManager::BufferPoolManager(DiskManager* dm){

    this->disk_manager = dm;
    this->lru = new LRU(BUFFER_SIZE);

    for(int frame_id{}; frame_id<BUFFER_SIZE;frame_id++){

        //for each frame, allocate a new empty frame on the heap
        this->frames[frame_id] = new char[PAGE_SIZE]; 
        this->is_dirty[frame_id] = false;
        this->free_frames_ids.push_back(frame_id);
        this->pages_ids[frame_id] = -1;
        this->pin_counter[frame_id] = 0;
    }    

}



char* BufferPoolManager::fetchPage(int page_id){

    if(page_id == -1){
        throw runtime_error("page id is -1");
    }
    
    int frame_id{-1};

    //check pages_table, if found , then return it 
    if(pages_table.find(page_id) != pages_table.end()){
        
        frame_id = pages_table[page_id];
        //fetch ur frame : 
        //char* frame = frames[frame_id];
        char* frame = lru->get_frame(frame_id);


        if(frame == nullptr){
            throw runtime_error("this frame is nullptr");
        }
        return frame;
    }

    cout<<"page is not found in pages_table, let's fetch from the HD"<<endl;
    //if not found on the pages_table, then we have to fetch it from the HD : 
        // if we have empty space for an extra page: 
            // then just use it, and fetch page
        // if no space, then evict a page, then replace it with the new page 

    //check if we have space : 
    if(free_frames_ids.empty() == true){
        //we have to evict a page :
        //LRU(comming soon), CLOCK.

        //stupid 
        //frame_id = BUFFER_SIZE - 1;
        frame_id = this->lru->evict_frame();
        
        //fetch this frame, and check if dirty 
        int old_page_id = pages_ids[frame_id];
        if(is_dirty[frame_id] == true){
            //write it to the hd 
            char* old_page_data = frames[frame_id];
            this->disk_manager->write_page(old_page_id, old_page_data);
        }
        //remove it from the pages_table 
        pages_table.erase(page_id);

    }
    //if we have empty frames, then just use one of them 
    else{
        frame_id = free_frames_ids.back();
        free_frames_ids.pop_back();
    }

    //we have a free frame that we can use:
    //then read ur page from the hd into this frame 
    char* buffer = frames[frame_id];
    this->disk_manager->read_page(page_id, buffer);

    //then update ur meta data: 
    pages_ids[frame_id] = page_id;
    is_dirty[frame_id] = false;
    pages_table[page_id] = frame_id;

    this->lru->put_node(frame_id, buffer);
    //then return ur result
    return buffer;

}

int BufferPoolManager::newPage(){

    int frame_id{-1};

    //check for free frames : 
    if(free_frames_ids.empty() ==true){
        //we have to evict a page 
        //frame_id = BUFFER_SIZE - 1;
        frame_id = this->lru->evict_frame();

        //fetch the evicted frame, then check if dirty 
        
        int old_page_id = pages_ids[frame_id];

        //write back to HD if dirty
        if(is_dirty[frame_id] == true){
            char* old_page_data = frames[frame_id];
            this->disk_manager->write_page(old_page_id, old_page_data);
        }

        this->pages_table.erase(old_page_id);
    }
    else{
        frame_id = free_frames_ids.back();
        free_frames_ids.pop_back();
    }

    // now we have a free frame, ready to store a new page
    // ask the disk_manager to allocate us a new page 
    int new_page_offset = this->disk_manager->allocate_page();
    int new_page_id = new_page_offset / PAGE_SIZE;

    //read it into our free frame 
    this->disk_manager->read_page(new_page_id, frames[frame_id]);

    //update the tables : 
    pages_ids[frame_id] = new_page_id;
    pages_table[new_page_id] = frame_id;
    is_dirty[frame_id] = true;

    this->lru->put_node(frame_id, frames[frame_id]);
    return new_page_id;
}

void BufferPoolManager::deletePage(int page_id){

    //check for the page in the pages_table 
    if(this->pages_table.find(page_id) != pages_table.end()){

        int frame_id = pages_table[page_id];
        //check if dirty before we remove it 
        if(is_dirty[frame_id] == true){
            this->disk_manager->write_page(page_id, frames[frame_id]);
        }

        //then update our meta-data: 
        pages_table.erase(page_id);
        pages_ids[frame_id] = -1;
        is_dirty[frame_id] =false;
        free_frames_ids.push_back(page_id);

        this->lru->remove_frame(frame_id);
    }
}

void BufferPoolManager::mark_as_dirty(int page_id){
    int frame_id = pages_table[page_id];
    is_dirty[frame_id] = true;
}


BufferPoolManager::~BufferPoolManager(){
    cout<<"deleting BPM"<<endl;
    for(int i{};i<BUFFER_SIZE;i++){

        int page_id = pages_ids[i];

        if(pin_counter[i] > 0) {
            cerr <<"Warning: Page"<<pages_ids[i] 
                      <<" still has "<<pin_counter[i]<<" pins during shutdown."<<endl;
        }

        if(is_dirty[i] && page_id != -1)
            disk_manager->write_page(page_id, frames[i]);

        if(frames[i]!=nullptr)
            delete[] frames[i];
    }
    this->disk_manager->~DiskManager();
    cout<<"BPM is deleted"<<endl;

}
/*
int 
main(){

    const string file_name = "DB.bin";
    DiskManager* dm = new DiskManager(file_name);


    {

        vector<Field>fields;

    {
        FieldType type = FieldType::TYPE_INT;
        int value = 9;

        Field f(type, value);

        fields.push_back(f);
    }
    {
        FieldType type = FieldType::TYPE_DOUBLE;
        double value = 9.1234;

        Field f(type, value);
        fields.push_back(f);
        
    }
    {
        FieldType type = FieldType::TYPE_STRING;
        const char* value = "ELFEEL";

        Field f(type, value);
        fields.push_back(f);
        
    }

        //create the tuple :
        Tuple tuple(fields);

        int page_id = 5;
        Page page(page_id);

        page.insert_tuple(tuple);
        page.insert_tuple(tuple);
        page.insert_tuple(tuple);
        page.insert_tuple(tuple);
        page.insert_tuple(tuple);


        cout<<"testing write page to the file "<<endl;
        {   
            dm->write_page(page_id, page.getData());
            dm->write_page(page_id+1, page.getData());
            cout<<"number of pages on the file "<< dm->get_number_of_pages()<<endl;

            cout<<"pages ids : "<<endl;
            for(auto& page_id : dm->get_all_pages_ids()){
                cout<<page_id<<" ";
            }
            cout<<endl;
        }

        BufferPoolManager* BPM = new BufferPoolManager(dm);
        cout<<"testing buffer pool manager"<<endl;
        cout<<"testing fetch page from BPM"<<endl;
        {   
            char* buffer = BPM->fetchPage(page_id);

            PageHeader* header = reinterpret_cast<PageHeader*>(buffer);
            cout<<"number of tuples "<<header->num_of_tuples<<endl;

            assert(header->num_of_tuples == 5);
        }
        cout<<"testing new page from BPM"<<endl;

        {
            int new_page_id = BPM->newPage();
            //read it
            char* buffer = BPM->fetchPage(new_page_id);
            PageHeader* header = reinterpret_cast<PageHeader*>(buffer);

            cout<<header->page_id<<endl;

            assert(header->page_id == new_page_id);
        }

        cout<<"testing delete page from BPM"<<endl;
        {
            //create a new page, then i will delete it
            int new_page_id = BPM->newPage();
            BPM->deletePage(new_page_id);
            char* buffer = BPM->fetchPage(new_page_id);

        }

    }

}
*/