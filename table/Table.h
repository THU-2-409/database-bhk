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
class VarPage;



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
    TableInfo(): header("", vector<ColDef>()) {}
    void make();
    
private:
    ByteArray dump();
    int load(char*);
public:
    void loadFrom();
    bool writeBack();

public:
    int RID;

public: // 无需存储又常用的值
    int recordLen;
    int dataPageRoom;
    int nullLength;

public:
    int getRecordLen() { return recordLen; }
};


class Trash {
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
                    oi = i;
                    break;
                }
            }
        }
    bool next();
    RecordData getData();
    void setData(RecordData rd);
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
    void remove(int offset);
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

public:
    DataPage(int page, Table * table);

    Record getRecord(int index);
    Record first();
    DataPage next();
};


class VarPage {
};


class Table {
private:
    TableInfo info;
public:
    static bool createFile(TableHeader header, string path);
    static int deleteFile(string path);
    bool open(string path);
    int close();

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

ByteArray TableHeader:: dump()
{
    char *buf = new char[PAGE_SIZE];
    MemOStream os;
    os.load(buf);
    os.putString(schema);
    os.putInt(defs.size());
    for (int i = 0; i < defs.size(); ++i)
    {
        os.putString(getName(i));
        os.putInt(getSize(i));
        os.putInt(getType(i));
    }
    // 写完
    ByteArray res(buf, os.length());
    delete[] buf;
    return res;
}

int TableHeader:: load(char *buf)
{
    MemIStream is;
    is.load(buf);
    is.getString(schema);
    int colNums;
    is.getInt(colNums);
    defs.clear();
    defs.insert(defs.end(), colNums, ColDef());
    for (int i = 0; i < colNums; ++i)
    {
        is.getString(defs[i].name);
        is.getInt(defs[i].size);
        is.getInt(defs[i].type);
    }
    return is.length();
}

HardManager* HardManager::getInstance()
{
    static HardManager *instance = new HardManager();
    return instance;
}

bool HardManager::createFile(string path)
{
    return fileManager.createFile(path.c_str());
}
int HardManager::deleteFile(string path)
{
    return remove(path.c_str());
}
bool HardManager::open(string path)
{
    return fileManager.openFile(path.c_str(), fileID);
}
int HardManager::close()
{
    bufPageManager.close();
    return fileManager.closeFile(fileID);
}

bool HardManager::_preAccess(int page)
{
    if (page < 0) return false;
    if (pages.size() <= page)
    {
        int eld = pages.size() - 1;
        pages.insert(pages.end(), page - eld, NULL);
        pageIndexs.insert(pageIndexs.end(), page - eld, 0);
    }
    pages[page] = (char*) bufPageManager.getPage(fileID
            , page, pageIndexs[page]);
    return true;
}

char* HardManager::getChars(int page, int offset, int size)
{
    assert(offset + size <= PAGE_SIZE);
    assert(_preAccess(page));
    bufPageManager.access(pageIndexs[page]);
    return pages[page] + offset;
}
bool HardManager::setChars(int page, int offset, const void *buf, int size)
{
    if (offset + size > PAGE_SIZE) return false;
    if (!_preAccess(page)) return false;
    bufPageManager.markDirty(pageIndexs[page]);
    memcpy(pages[page] + offset, buf, size);
    return true;
}
int HardManager::getInt(int page, int offset)
{
    int *x = (int*) this->getChars(page, offset, 4);
    return *x;
}
string HardManager::getString(int page, int offset)
{
    // 字符串在文件中以\0结尾
    char *buf = this->getChars(page, offset, 1);
    return string(buf);
}
bool HardManager::setInt(int page, int offset, int val)
{
    return setChars(page, offset, &val, 4);
}
bool HardManager::setString(int page, int offset, string str)
{
    // \0 也要写入文件
    return setChars(page, offset, str.c_str(), str.size() + 1);
}

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
