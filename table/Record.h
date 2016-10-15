//
// Created by kinnplh on 2016/10/15.
//

#ifndef RECORD_H
#define RECORD_H

#include <string>
#include "../utils/pagedef.h"

using namespace std;
class Record {
    int page;
    union {
        int offset;
        BufType* address;
    };
    Table* table;
    friend class Table;
public:
    int getInt(string);
    string getString(string);
    void setInt(string);
    void setString(string);

    Record(int _page = -1, int _offset = -1);

};


#endif //DATABASE_BHK_RECORD_H
