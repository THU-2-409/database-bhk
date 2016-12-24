#ifndef _H_DB_TABLE_
#define _H_DB_TABLE_

#include "../utils.h"
#include "../utils/MemStream.h"
#include "../fileio/FileManager.h"
#include "../bufmanager/BufPageManager.h"
#include <utility>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

#include "../table/ColDef.h"

using namespace std;

class TableHeader;
class HardManager;
class TableInfo;
class Trash;
class Record;
class InvertedIndexArray;
class RecordData;
class DataPage;



class TableHeader {
private:
    string schema;
    map<string, int> dict;
    vector<ColDef> defs;
public:
    TableHeader(string schema, vector<ColDef> defs)
        :schema(schema), defs(defs)
    {
        for (int i = 0; i < defs.size(); ++i)
        {
            dict[defs[i].name] = i;
        }
    }

    string getSchema() const { return this->schema; }
    int getColNums() const { return defs.size(); }
    string getName(int col) const { return defs.at(col).name; }
    int getSize(int col) const { return defs.at(col).size; }
    int getType(int col) const { return defs.at(col).type; }
    int getConstraint(int col) const { return defs.at(col).constraint; }
    
    ByteArray dump();
    int load(char*);
};


class HardManager {
private:
    FileManager fileManager;
    BufPageManager bufPageManager;
    int fileID;
    vector<char*> pages;
    vector<int> pageIndexs;
private:
    HardManager(): bufPageManager(&fileManager) {}
    bool _preAccess(int page);
public:
    static HardManager* getInstance();
public:
    bool createFile(string path);
    int deleteFile(string path);
    bool open(string path);
    int close();
public:
    char* getChars(int page, int offset, int size);
    bool setChars(int page, int offset, const void *buf, int size);
    int getInt(int page, int offset);
    string getString(int page, int offset);
    bool setInt(int page, int offset, int val);
    bool setString(int page, int offset, string str);
};


class TableInfo {
public:
    TableHeader header;
    TableInfo();
    void make();
    
private:
    ByteArray dump();
    int load(char*);
public:
    void loadFrom();
    bool writeBack();

public:
    int RID;
    int nextNewPage;
    int nextNewRecPage;
    int nextNewRecOffset;
    int maxPageID;

public: // 无需存储又常用的值
    int recordLen;
    int dataPageRoom;
    int nullLength;

public:
    int getRecordLen() { return recordLen; }
};


class Trash {
private:
    Table * table;
public:
    Trash(Table * table);
    DataPage allocPage();
    Record allocRecord();
    void freePage(int page);
    void freeRecord(int page, int offset);
};


class Record {
public:
    Record(int p, int o, TableInfo* t,vector<int> vv)
        :page(p),offset(o),info(t),v(vv) 
        {
            int size = v.size();
            for(int i = 0; i < size; i++)
            {
                if(v[i] == o)
                {
                    oindex = i;
                    break;
                }
            }
        }
    bool next();
    RecordData getData();
    void setData(RecordData rd);
    int getPageID() { return page; }
    int getOffset() { return offset; }
private:
    int page;
    int offset;
    int oindex;
    TableInfo *info;
    vector<int> v;
};

class InvertedIndexArray {
public:
    InvertedIndexArray(int p):page(p){};
    void push(int offset);
    int remove(int offset);
    vector<int> getVector();
private:
    int page;
};

class RecordData {
 public:
    pair<bool, int> getInt(string fname);//fieldname 属性名
    void setInt(string fname, int value);

    pair<bool, string> getString(string fname);
    void setString(string fname, string value);

    void setNULL(string fname);

    pair<bool, ByteArray> getBA(string fname);
    void setBA(string fname, ByteArray value);

private:
    map< string, pair<bool, ByteArray> > m;
};


class DataPage {
private:
    int page; // 页号
    Table * table;
    InvertedIndexArray * invArr;

public:
    DataPage(int page, Table * table);
    ~DataPage();

    int getPageID() { return page; }
    Record getRecord(int index);
    Record first();
    DataPage next();

    void removeRecord(int offset);
    void removeRecord(Record rec)
    {
        if (rec.getPageID() == page)
            this->removeRecord(rec.getOffset());
    }
};


class Table {
private:
    TableInfo info;
    Trash trash;
public:
    Table(): trash(this) {}
    static bool createFile(TableHeader header, string path);
    static int deleteFile(string path);
    bool open(string path);
    int close();

    void insert(RecordData data);
    vector<Record> find(RecordData data);

    string getSchema();

    friend class Datapage;
    friend class Record;
    friend class RecordData;
};



// 以下为实现
#include "RecordData.cpp"
#include "InvertedIndexArray.cpp"
#include "Record.cpp"

#include "../table/DataPage.cpp"

#include "../table/TableInfo.cpp"

#include "../table/Trash.cpp"

#include "../table/TableHeader.cpp"

#include "../table/HardManager.cpp"

#include "../table/Table.cpp"

bool Table::createFile(TableHeader header, string path)
{
    HardManager *hard = HardManager::getInstance();
    if (!hard->createFile(path)) return false;
    if (!hard->open(path)) return false;
    Table table;
    table.info.header = header;
    table.info.writeBack();
    return true;
}
int Table::deleteFile(string path)
{
    HardManager *hard = HardManager::getInstance();
    return hard->deleteFile(path);
}
bool Table::open(string path)
{
    HardManager *hard = HardManager::getInstance();
    if (!hard->open(path)) return false;
    this->info.loadFrom();
    return true;
}
int Table::close()
{
    if (!this->info.writeBack()) return 1;
    HardManager *hard = HardManager::getInstance();
    return hard->close();
}

string Table:: getSchema()
{
    return info.header.getSchema();
}

#endif
