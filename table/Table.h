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
#define PAGE_SIZE 8192
using namespace std;

class Table {
public:
    Table():memPageTop(0),bufPageManager(&fileManager) {}
    friend class Record;
    static pair<bool, Table> createFile(TableHeader header, string path);
    static int deleteFile(string path);
    bool open(string path);
    bool close();
    Record insertRecord(Record record);
    bool deleteRecord(Record record);
    bool updateRecord(Record real, Record dummy);
    list<Record> selectRecord(Record cond);

	TableHeader th;
private:
    char* getChars(int page, int offset, int size);
    bool setChars(int page, int offset, char* buf, int size);
    int newMemPage();
    void releaseMemPage(int page);

    // 非接口
    bool _preAccess(int page); // true表示正常
    void _writeInfo2Hard(); // 将第一页信息写入文件系统

    FileManager fileManager;
    BufPageManager bufPageManager;
    map<int,char*> memPages;
    vector<char*> hardPages;
    int memPageTop;
	int FileID;
    // 第一页,表信息，内存备份
    int recordLength, numOfColumns, recordNumber, pageNumber;
    vector<int> recordNumOfPage, availOfPage;
};

void Table::_writeInfo2Hard() // 将第一页信息写入文件系统
{
    char* buf = new char[PAGE_SIZE];
    int offset = 0;
    // 写入缓存
    memcpy(buf, &recordLength, sizeof(int)); // 记录长度
    offset += sizeof(int);
    memcpy(buf + offset, &numOfColumns, sizeof(int)); // 列数
    offset += sizeof(int);
    for (int i = 0; i < th.getSize(); ++i) // 写入Header.second
    {
        memcpy(buf + offset, &(th[i].second), sizeof(int));
        offset += sizeof(int);
    }
    for (int i = 0; i < th.getSize(); ++i) // 写入Header.first
    {
        tmp = th[i].first.length();
        memcpy(buf + offset, &tmp, sizeof(int)); // 写入Header.first.length
        offset += sizeof(int);
        memcpy(buf + offset, th[i].first.c_str(), tmp); // 写入Header.first
        offset += tmp;
    }
    memcpy(buf + offset, &recordNumber, sizeof(int)); // 写入记录总数
    offset += sizeof(int);
    memcpy(buf + offset, &pageNumber, sizeof(int)); // 写入页数
    offset += sizeof(int);
    for (int i = 0; i < pageNumber; ++i)
    {
        memcpy(buf + offset, &recordNumOfPage[i], sizeof(int)); // 每一页的记录个数
        offset += sizeof(int);
        memcpy(buf + offset, &availOfPage[i], sizeof(int)); // 每一页的第一个空位
        offset += sizeof(int);
    }

    // 写入文件系统
    table.setChars(0, 0, buf, offset);
}

pair<bool, Table> Table::createFile(TableHeader header, string path) 
{
    Table table;
    if (!fileManager.createFile(path.c_str())) return make_pair(false, table);
    table.th = header;
    if (!table.open(path)) return make_pair(false, table);
    int offset = 0;
    int tmp = 0;
    // 计算信息
    table.recordLength = 0;
    table.numOfColumns = header.getSize();
    table.recordNumber = 0;
    table.pageNumber = 1;
    table.recordNumOfPage = vector<int>();
    table.availOfPage = vector<int>();
    for (int i = 0; i < header.getSize(); ++i)
        table.recordLength += header[i].second;
    // 写入
    _writeInfo2Hard();
}

bool Table::_preAccess(int page)
{
    if (page >= 0) 
    {
        if (hardPages.size() <= page)
        {
            hardPages.insert(hardPages.end(), page - hardPages.size() + 1, NULL);
        }
        if (hardPages[page] == NULL)
        {
            int tmp;
            hardPages[page] = (char*) bufPageManager.getPage(FileID, page, tmp);
        }
    } else
        return memPages.find(page) != memPages.end();
    return true;
}

char* Table::getChars(int page, int offset, int size)
{
    if (offset + size > PAGE_SIZE) return NULL;
    if (!_preAccess(page)) return NULL;
    char *buf;
    if (page >= 0) buf = hardPages[page];
    else buf = memPages[page];
    return buf + offset;
}

bool setChars(int page, int offset, char *buf, int size)
{
    char *ref;
    if (page >= 0) 
    {
        if (offset + size > PAGE_SIZE) return false;
        _preAccess(page);
        ref = hardPages[page];
    }
    else 
    {
        if (offset + size > recordLength) return false;
        if (memPages.find(page) == memPages.end())
        {
            ref = new char[recordLength];
            memPages[page] = ref;
        } else
            ref = memPages[page];
    }
    memcpy(ref + offset, buf, size);
}

int Table::deleteFile(string path){
	return remove(path);
}

#endif
// vim: ts=4
