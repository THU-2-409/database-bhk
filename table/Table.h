#ifndef _H_DB_TABLE_
#define _H_DB_TABLE_

#include "../utils/pagedef.h"
#include "../fileio/FileManager.h"
#include "../bufmanager/BufPageManager.h"
#include "TableHeader.h"
#include "Record.h"
#include <utility>
#include <string>
#include <list>
using namespace std;

class Table {
public:
    Table() {}
    friend class Record;
    static pair<bool, Table> createFile(TableHeader header, string path);
    static bool deleteFile(string path);
    Record insertRecord(Record record);
    bool deleteRecord(Record record);
    bool updateRecord(Record real, Record dummy);
    list<Record> selectRecord(Record cond);
private:
    BufType getBuf(int page, int offset, int size);
    bool setBuf(int page, int offset, BufType buf, int size);

    FileManager fileManager;
};

static pair<bool, Table> Table::createFile(TableHeader header, string path) 
{
    if ()
}

#endif
