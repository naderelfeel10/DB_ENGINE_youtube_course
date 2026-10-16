#pragma once

#include"Storage/Page/Tuple.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include<string>

using namespace std;

class Index{

    public:
        virtual void Insert(Field& field, string col_name, vector<Column> schema, RID rid)=0;
        virtual void Delete (Field& field)=0;
        virtual vector<RID> Search(Field& field)=0;
        
};