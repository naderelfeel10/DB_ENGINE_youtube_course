#pragma once 


#include"Storage/Page/Page.h"
#include<unordered_map>
#include"fstream"
#define CAPACITY 100


struct DirEntry{
    int page_id;
    size_t offset;
};

class DiskManager{

    // file_name
    string file_name;
    //page_dir that maps page_id to the offset on file
    unordered_map<int, size_t> pages_dir;
    //the actual file 
    fstream DB_file;

    int capacity = CAPACITY;

    vector<size_t> deleted_pages;

public:
    DiskManager(const string& file_name);

    // crud 
    void write_page(int page_id, char* data);
    void read_page(int page_id, char* data);
    void delete_page(int page_id);
    size_t allocate_page();

    void resizeFile();

    int get_number_of_pages();
    vector<int> get_all_pages_ids();


    void save_dm();
    void load_dm();


};
