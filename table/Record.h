//
// Created by kinnplh on 2016/10/15.
//

#ifndef RECORD_H
#define RECORD_H

#include <string>
#include "../utils/pagedef.h"
#include "Table.h"
using namespace std;
class Record {
    int page;
    union {
        int offset;
        char* address;
    };
    Table* table;
    friend class Table;
    void switchBtwImgReal();
public:
    int getInt(string);
    string getString(string);
    bool setInt(string, int);
    bool setString(string, string);

    Record(Table* _table, int _page = -1, int _offset = -1);
};


#endif //DATABASE_BHK_RECORD_H
