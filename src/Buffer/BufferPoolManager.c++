#include"Buffer/BufferPoolManager.h"


BufferPoolManager::BufferPoolManager(DiskManager* dm){
    
    this->disk_manager = dm;   
    this->lru = new LRU(BUFFER_SIZE);

    //setup BPM : 
    for(int i{}; i<BUFFER_SIZE; i++){

        this->frames[i] = new char[PAGE_SIZE];
        this->is_dirty[i] = false;
        this->free_framses_ids.push_back(i);
        this->pin_count[i] = 0;
        this->pages_ids[i] = -1;
    }

}

char* BufferPoolManager::fetchPage(int page_id){

    //check page_id
    if(page_id == -1){
        cout<<"can't fetch page with page_di = -1";
        return nullptr;
    }
   
    int frame_id{-1};

    //check if the page is in  the pages_table
    if(pages_table.find(page_id) != pages_table.end()){
        
        frame_id = pages_table[page_id];
        
        //char* frame = lru->get_frame(frame_id);
        char* frame = frames[page_id];
        
        if(frame == nullptr){
            throw runtime_error("frame is null");
        }
        
        return frame;
    }

    cout<<"fetching the page from HD"<<endl;
    //if it's not found in the pages_table
    // then we have to fetch it from the disk

    //we have 2 cases : 
        // 1. if there is room for additional page
            // then I will just read the page from the hard disk
        // 2. else if all frames are occupied 
            // then we have to evict a page then read the new one
    
    
    // if no free frames
    if(free_framses_ids.empty() == true){

        // for now just evict the last frame

        frame_id = BUFFER_SIZE -1;

        //frame_id = this->lru->evict_frame();

        //save the old page, then replace it with the new one 
        int old_page_id = pages_ids[frame_id];
        
        /*
        //if is dirty, then save it before evicting
        if(is_dirty[frame_id]){

            char* old_page_data = frames[frame_id];
            this->disk_manager->write_page(old_page_id, old_page_data);
        
        }

        pages_table.erase(old_page_id);
        */

    }
    //else if there is still free frames to use 
    //then just use the last free frame
    else{
        frame_id = free_framses_ids.back();
        free_framses_ids.pop_back();
    }

    // now we have an empty frame to read the data 
    char* buffer = frames[frame_id];
    disk_manager->read_page(page_id, buffer);

    //now update our buffer pool
    pages_ids[frame_id] = page_id;
    is_dirty[frame_id] = false;
    pages_table[page_id] = frame_id;

    //finally return the page
    char* res_buffer = frames[frame_id];

    //lru->put_frame(frame_id, frames[frame_id]);
    
    return res_buffer;
}


int BufferPoolManager::newPage(){
    
    int frame_id{-1};
    
    // if no free frames
    if(free_framses_ids.empty() == true){

        // for now just evict the last frame
        frame_id = BUFFER_SIZE -1;
        
        //frame_id  = lru->evict_frame();

        //save the old page, then replace it with the new one 
        int old_page_id = pages_ids[frame_id];
        
        this->deletePage(old_page_id);
        /*
        //if is dirty, then save it before evicting
        if(is_dirty[frame_id]){

            char* old_page_data = frames[frame_id];
            this->disk_manager->write_page(old_page_id, old_page_data);
        
        }

        pages_table.erase(old_page_id);
        */
    }
    //else if there is still free frames to use 
    //then just use the last free frame
    else{
        frame_id = free_framses_ids.back();
        free_framses_ids.pop_back();
    }

    //now create a new page on disk : 
    
    int new_page_offset = this->disk_manager->allocate_page();
    int new_page_id = new_page_offset / PAGE_SIZE;

    //then read this page : 
    char* free_frame_buffer = frames[frame_id];
    this->disk_manager->read_page(new_page_id, frames[frame_id]);
    
    PageHeader* header = reinterpret_cast<PageHeader*>(frames[frame_id]);
    cout<<header->page_id<<endl;
    //header->page_id = 

    // then initialize page_header : 

    //update buffer pool tables 
    pages_ids[frame_id] = new_page_id;
    pages_table[new_page_id] = frame_id;
    is_dirty[frame_id] = false;

    //lru->put_frame(frame_id, frames[frame_id]);

    return new_page_id;

}

void BufferPoolManager::deletePage(int page_id){
    
    //check if page id exists in pages_table : 
    if(pages_table.find(page_id) != pages_table.end()){

        int frame_id = pages_table[page_id];
        //flush to disk if dirty : 
        if(is_dirty[frame_id] == true){
            this->disk_manager->write_page(page_id, frames[frame_id]);
        }
        //then remove from BPM tables : 
        pages_table.erase(page_id);
        pages_ids[frame_id] = -1;
        is_dirty[frame_id] = false;
        free_framses_ids.push_back(frame_id);

        //lru->remove_frame(frame_id);
    }
}

void BufferPoolManager::mark_as_dirty(int page_id){
    int frame_id = pages_table[page_id];
    is_dirty[frame_id] = true;
}

void BufferPoolManager::unpin_page(int page_id, bool is_dirty_flag){

    //check it it exists in pages_table first : 
    if(pages_table.find(page_id) == pages_table.end()){
        return ;
    }

    int frame_id = pages_ids[page_id];
    
    //decrease pin cound
    if(pin_count[frame_id] > 0){
        pin_count[frame_id]--;
    }
    //mark as dirty
    if(is_dirty_flag){
        is_dirty[frame_id] = true;
    }
}
// ovsrervability of llms : 
  // 1. langfuse
  // 2. 

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

        int page_id = 1;
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

        cout<<"testing buffer pool manager"<<endl;

        BufferPoolManager* BPM = new BufferPoolManager(dm);

        cout<<"tesging fetch page"<<endl;
        {
            char* page_buffer = BPM->fetchPage(page_id);

            PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer);
        
            cout<<"number of tuples "<<page_header->num_of_tuples<<endl;

            assert(page_header->num_of_tuples == 5);
        }

        cout<<"tesging new page"<<endl;
        {
            int new_page_id = BPM->newPage();
            //cout<<"new_page_id"<<new_page_id<<endl;
            char* page_buffer = BPM->fetchPage(new_page_id);
            //char* page_buffer = new char[PAGE_SIZE];
            dm->read_page(new_page_id, page_buffer);
            PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer);
        
            cout<<"new page id "<<page_header->page_id<<endl;

            assert(page_header->page_id == new_page_id);
        }

        cout<<"testing delete page"<<endl;
        {
            int new_page_id = BPM->newPage();
            BPM->deletePage(new_page_id);
            BPM->fetchPage(new_page_id);

        }
}

}
