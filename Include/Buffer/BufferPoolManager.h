#pragma once 

#include"Storage/Disk/disk_manger.h"
#include"Buffer/LRU.h"


#include<vector>
#include<unordered_map>

#define BUFFER_SIZE 3
using namespace std;

class BufferPoolManager{

private:
    DiskManager* disk_manager;
    LRU* lru;
    char* frames[BUFFER_SIZE];
    unordered_map<int, size_t> pages_table;

    bool is_dirty[BUFFER_SIZE];
    vector<int> free_framses_ids;
    int pages_ids[BUFFER_SIZE];
    int pin_count[BUFFER_SIZE];

public:
    
    BufferPoolManager(DiskManager* dm);
    
    //crud 
    char* fetchPage(int page_id);
    int newPage();
    void deletePage(int page_id);

    void mark_as_dirty(int page_id);
    void unpin_page(int page_id, bool is_dirty_flag);
};