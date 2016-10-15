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
#include <stdio.h>
using namespace std;

class Table {
	TableHeader th;
	int FileID;
public:
    Table() {}
    friend class Record;
    static pair<bool, Table> createFile(TableHeader header, string path);
    static int deleteFile(string path);
    bool open(string path);
    bool close();
    Record insertRecord(Record record);
    bool deleteRecord(Record record);
    bool updateRecord(Record real, Record dummy);
    list<Record> selectRecord(Record cond);
private:
    BufType getBuf(int page, int offset, int size);
    bool setBuf(int page, int offset, BufType buf, int size);

    FileManager fileManager;
};

pair<bool, Table> Table::createFile(TableHeader header, string path) 
{
    Table table;
    if (!fileManager.createFile(path.c_str())) return make_pair(false, table);
    table.th = header;
    if (!table.open(path)) return make_pair(false, table);
    int offset = 0;
}

int Table::deleteFile(string path){
	return remove(path);
}

#endif
// vim: ts=4
