#pragma once 


#define BUFFER_SIZE 100

#include<unordered_map>
#include<vector>
#include"Storage/Disk/disk_manger.h"
#include"LRU.h"

using namespace std;

class BufferPoolManager{

private : 

    DiskManager* disk_manager;
    LRU* lru;
    //frames arr : 
    char* frames[BUFFER_SIZE];
    //pages_table : page_id -> frame_id
    unordered_map<int, size_t> pages_table;
    //is dirty pages 
    bool is_dirty[BUFFER_SIZE];
    //track free frames ids
    vector<int> free_frames_ids;
    // map frame_id -> page_id
    int pages_ids[BUFFER_SIZE];
    //pin counter arr
    int pin_counter[BUFFER_SIZE];


public:
    BufferPoolManager(DiskManager* dm);

    //crud : 
    char* fetchPage(int page_id);
    int newPage();
    void deletePage(int page_id);

    void mark_as_dirty(int page_id);
    ~BufferPoolManager();
};