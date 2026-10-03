#include"Storage/Disk/disk_manger.h"
#include<filesystem>

DiskManager::DiskManager(const string& file_name){

    this->file_name = file_name;

    //try to open this file in read-write mode
    this->DB_file.open(file_name , ios::binary | ios::in | ios::out);

    //if the file does not exist
    // create new one, and open 
    if(DB_file.is_open() == false){

        this->DB_file.open(file_name , ios::binary | ios::trunc |  ios::in | ios::out);

        //then resize the file 
        int new_size = capacity * PAGE_SIZE;
        filesystem::resize_file(file_name, new_size);
    
        cout<<"new file is created "<<endl;
        this->save_dm();
    
    }else{
        this->load_dm();
        cout<<"this file is openned successfuly "<<endl;
    }



}

void DiskManager::write_page(int page_id, char* data){

    size_t offset{0};
    //check if this page is in page_dir
    if(this->pages_dir.find(page_id) != pages_dir.end()){
        offset = pages_dir[page_id];
    }else{
        // if the page is not in page_dir
        //allocate new one 
        
        offset = allocate_page();
    }

    //move the write pointer to the offset 
    DB_file.seekp(offset);
    DB_file.write(data, PAGE_SIZE);

    //then update our page_dir
    this->pages_dir[page_id] = offset;

    //flush to the disk
    DB_file.flush();

}   



void DiskManager::read_page(int page_id, char* data){

    //check the pages_dir
    if(this->pages_dir.find(page_id) == pages_dir.end()){
        cout<<"this page is not found"<<endl;
        return;
    }

    // if found : 
    size_t offset = pages_dir[page_id];
    DB_file.seekg(offset, ios::beg);

    if(DB_file.fail()){
        throw runtime_error("failed to move the pointer");
    }

    DB_file.read(data, PAGE_SIZE);

    if(DB_file.fail()){
        throw runtime_error("failed to read the data");
    }

}


void DiskManager::delete_page(int page_id){
    //check the pages_dir
    if(this->pages_dir.find(page_id) == pages_dir.end()){
        cout<<"this page is not found"<<endl;
        return;
    }

    // soft delete
    size_t offset = pages_dir[page_id];
    this->deleted_pages.push_back(offset);

    //remove it from the pages_dir
    pages_dir.erase(page_id);

    cout<<"page is deleted"<<endl;

}

void DiskManager::resizeFile(){
    
    //increase the capacity
    this->capacity *=2;
    int new_size = this->capacity * PAGE_SIZE;
    filesystem::resize_file(file_name, new_size);

}

size_t DiskManager::allocate_page(){

    //check for the deleted pages first
    if(deleted_pages.empty() == false){
        size_t last_offset = deleted_pages.back();
        deleted_pages.pop_back();

        return last_offset;
    }

    //check if there is enough space in the file
    //if not, then resize
    if(this->pages_dir.size() + 1 >= this->capacity ){
        resizeFile();
    }

    //calc the page_id 
    // create a new page with this page_id
    int page_id = this->pages_dir.size() + 1;
    Page* new_page = new Page(page_id);

    PageHeader* header = reinterpret_cast<PageHeader*>(new_page->getData());
    cout<<header->page_id<<endl;
    header->page_id = page_id;

    //then write it on the file
    size_t offset = page_id * PAGE_SIZE;

    DB_file.seekp(offset);
    DB_file.write(new_page->getData(), PAGE_SIZE);
    DB_file.flush();

    pages_dir[page_id] = offset;

    delete new_page;

    return offset;

}

int DiskManager::get_number_of_pages(){
    return this->pages_dir.size();
}

vector<int> DiskManager::get_all_pages_ids(){

    vector<int> res;
    for(auto& [page_id, offset] : this->pages_dir){
        res.push_back(page_id);
    }

    return res;
}


void DiskManager::save_dm(){

    cout<<"saving meta data"<<endl;

    int offset{0};
    char buffer[PAGE_SIZE];
    memset(buffer,0,PAGE_SIZE);

    //save page capacity
    memcpy(buffer+offset, &this->capacity, sizeof(capacity));
    offset += sizeof(capacity);


    //save file_name 
    int file_name_size = this->file_name.size();
    memcpy(buffer+offset, &file_name_size, sizeof(file_name_size));
    offset += sizeof(file_name_size);

    memcpy(buffer+offset, file_name.c_str(), file_name_size);
    offset += file_name_size;

    //save pages dir

    int pages_dir_size = this->pages_dir.size();
    memcpy(buffer+offset, &pages_dir_size, sizeof(pages_dir_size));
    offset += sizeof(pages_dir_size);


    for(auto& [page_id, off] : this->pages_dir){
        
        DirEntry entry = {page_id, off};
        memcpy(buffer+offset, &entry, sizeof(entry));
        offset += sizeof(entry);
    }


    int deleted_pages_size = this->deleted_pages.size();
    memcpy(buffer+offset, &deleted_pages_size, sizeof(deleted_pages_size));
    offset += sizeof(deleted_pages_size);

    //save deleted_pages
    for(size_t off: deleted_pages){

        memcpy(buffer+offset, &off, sizeof(off));
        offset += sizeof(off);

    }

    //write this into the disk:

    
    DB_file.clear();
    DB_file.seekp(0, ios::beg);
    DB_file.write(buffer, PAGE_SIZE);

    //flush to the disk
    DB_file.flush();

}

void DiskManager::load_dm(){

    cout<<"loading the meta data of this file "<<endl;
    
    //read the first_page in the file into the buffer
    
    char buffer[PAGE_SIZE];
    
    DB_file.clear();
    DB_file.seekg(0, ios::beg);
    DB_file.read(buffer, PAGE_SIZE);

    int offset{0};

    //load page capacity
    memcpy(&this->capacity, buffer+offset, sizeof(capacity));
    offset += sizeof(capacity);

    cout<<this->capacity<<endl;
    
    //load file_name 
    int file_name_size;
    memcpy(&file_name_size,buffer+offset, sizeof(file_name_size));
    offset += sizeof(file_name_size);

    file_name.assign(buffer+offset, file_name_size);
    offset += file_name_size;

    cout<<this->file_name<<endl;

    //load pages dir
    int pages_dir_size;
    memcpy(&pages_dir_size, buffer+offset, sizeof(pages_dir_size));
    offset += sizeof(pages_dir_size);


    this->pages_dir.clear();
    for(int i=0;i<pages_dir_size; i++){
        DirEntry entry;
        memcpy(&entry, buffer+offset, sizeof(entry));
        offset += sizeof(entry);

        //upate pages_dir
        pages_dir[entry.page_id] = entry.offset;
    }


    //load deleted pages
    int deleted_pages_size;
    memcpy(&deleted_pages_size, buffer+offset, sizeof(deleted_pages_size));
    offset += sizeof(deleted_pages_size);

    this->deleted_pages.clear();

    for(int i=0;i<deleted_pages_size;i++){
        size_t off;
        memcpy(&off, buffer+offset, sizeof(off));
        offset += sizeof(off);

        //update deleted pages 
        this->deleted_pages.push_back(off);
    }


}

DiskManager::~DiskManager(){
    cout<<"saving dm"<<endl;
    if(DB_file.is_open()){
        DB_file.clear();
        save_dm();
        DB_file.close();
    }
    cout<<"dm is saved"<<endl;
}


/*
int 
main(){
    //test file creation 

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


        cout<<"testing read page from the file "<<endl;
        {   

            char* dummy_buffer = new char[PAGE_SIZE];
            dm->read_page(page_id, dummy_buffer);

            PageHeader* header = reinterpret_cast<PageHeader*>(dummy_buffer);
            cout<<"number of tuples "<<header->num_of_tuples<<endl;

            assert(header->num_of_tuples == 5);
        }


        cout<<"testing delete page from the file "<<endl;
        {   

            dm->delete_page(page_id);

            char* dummy_buffer = new char[PAGE_SIZE];
            dm->read_page(page_id, dummy_buffer);

        }
    }
    

}
*/