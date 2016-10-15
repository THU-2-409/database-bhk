#ifndef _H_DB_TABLE_
#define _H_DB_TABLE_

#include "../utils/pagedef.h"
#include "../fileio/FileManager.h"
#include "../bufmanager/BufPageManager.h"
#include "TableHeader.h"
#include "Record.h"
#include <utility>
#include <string>
#include <cstring>
#include <list>
#include <stdio.h>
using namespace std;

class Table {
	TableHeader th;
	int FileID;
public:
    Table():memPageTop(0) {}
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
    char* getChars(int page, int offset, int size);
    bool setChars(int page, int offset, char* buf, int size);
    int newMemPage();
    void releaseMemPage(int page);

    // 非接口
    bool _preAccess(int page); // true表示正常

    FileManager fileManager;
    map<int,char*> memPages;
    int memPageTop;
    // 第一页,表信息，内存备份
    int recordLength, numOfColumns, recordNumber, pageNumber;
    vector<int> recordNumOfPage, availOfPage;
};

pair<bool, Table> Table::createFile(TableHeader header, string path) 
{
    Table table;
    if (!fileManager.createFile(path.c_str())) return make_pair(false, table);
    table.th = header;
    if (!table.open(path)) return make_pair(false, table);
    int offset = 0;
    int tmp = 0;
    char* buf = new char[8 * 1024];
    // 计算信息
    table.recordLength = 0;
    table.numOfColumns = header.getSize();
    table.recordNumber = 0;
    table.pageNumber = 1;
    table.recordNumOfPage = vector<int>();
    table.availOfPage = vector<int>();
    for (int i = 0; i < header.getSize(); ++i)
        table.recordLength += header[i].second;
    // 写入缓存
    memcpy(buf, &table.recordLength, sizeof(int)); // 记录长度
    offset += sizeof(int);
    memcpy(buf + offset, &table.numOfColumns, sizeof(int)); // 列数
    offset += sizeof(int);
    for (int i = 0; i < header.getSize(); ++i) // 写入Header.second
    {
        memcpy(buf + offset, &(header[i].second), sizeof(int));
        offset += sizeof(int);
    }
    for (int i = 0; i < header.getSize(); ++i) // 写入Header.first
    {
        tmp = header[i].first.length();
        memcpy(buf + offset, &tmp, sizeof(int)); // 写入Header.first.length
        offset += sizeof(int);
        memcpy(buf + offset, header[i].first.c_str(), tmp); // 写入Header.first
        offset += tmp;
    }
    memcpy(buf + offset, &table.recordNumber, sizeof(int)); // 写入记录总数
    offset += sizeof(int);
    memcpy(buf + offset, &table.pageNumber, sizeof(int)); // 写入页数
    offset += sizeof(int);
    // 因为create的时候没有页，所以余下都是空的

    // 写入文件系统
    table.setChars(0, 0, buf, offset);
}

char* Table::getChars(int page, int offset, int size)
{
}

int Table::deleteFile(string path){
	return remove(path);
}

#endif
// vim: ts=4
