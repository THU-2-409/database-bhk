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
#include <vector>
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
    pair<bool,Record> selectRecord(Record cond);

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
    vector<int> bufPageIndex;
    int memPageTop;
	int FileID;
    // 第一页,表信息，内存备份
    int __RID__; // R!I!D!
    int recordLength, numOfColumns, recordNumber, pageNumber;
    vector<int> recordNumOfPage, availOfPage;

    static int check[256];
};

void Table::_writeInfo2Hard() // 将第一页信息写入文件系统
{
    char* buf = new char[PAGE_SIZE];
    int offset = 0;
    // 写入缓存
    memcpy(buf + offset, &__RID__, sizeof(int)); // R!I!D!
    offset += sizeof(int);
    memcpy(buf + offset, &recordLength, sizeof(int)); // 记录长度
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
        tmp = th[i].first.length() + 1;
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
    table._writeInfo2Hard();
}

bool Table::_preAccess(int page)
{
    if (page >= 0) 
    {
        if (hardPages.size() <= page)
        {
            int eld = hardPages.size() - 1;
            hardPages.insert(hardPages.end(), page - eld, NULL);
            bufPageIndex.insert(bufPageIndex.end(), page - eld, 0);
        }
        hardPages[page] = (char*) bufPageManager.getPage(FileID,
                page, bufPageIndex[page]);
    } else
        return memPages.find(page) != memPages.end();
    return true;
}

char* Table::getChars(int page, int offset, int size)
{
    if (offset + size > PAGE_SIZE) return NULL;
    if (!_preAccess(page)) return NULL;
    char *buf;
    if (page >= 0) 
    {
        buf = hardPages[page];
        bufPageManager.access(bufPageIndex[page]);
    }
    else buf = memPages[page];
    return buf + offset;
}

bool setChars(int page, int offset, char *buf, int size)
{
    if (size <= 0) return false;
    char *ref;
    if (page >= 0) 
    {
        if (offset + size > PAGE_SIZE) return false;
        _preAccess(page);
        ref = hardPages[page];
        bufPageManager.markDirty(bufPageIndex[page]);
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
    return true;
}

int Table::deleteFile(string path){
	return remove(path);
}

int Table::newMemPage()
{
    return ++ memPageTop;
}
void releaseMemPage(int page)
{
    if (page < 0 && memPages.find(page) != memPages.end())
    {
        delete[] memPages[page];
        memPages.erase(memPages.find(page));
    }
}

bool Table::deleteRecord(Record record)
{
    if (record.page < 0) return false;
    int index = record.offset / recordLength;
    int row = index >> 3;
    int col = index & 7;
    char *bitB = getChars(record.page, PAGE_SIZE - row - 1, 1);
    if ((*bitB & (1 << col)) == 0) return false; // 0 未使用，小端存储
    char tmp = *bitB;
    tmp ^= (1 << col);
    -- recordNumOfPage[record.page];
    if (availOfPage[record.page] == -1)
        availOfPage[record.page] = index;
    return setChars(record.page, PAGE_SIZE - row - 1, &tmp, 1);
}

bool Table::open(string path) {
    //read:
    //int recordLength, numOfColumns, recordNumber, pageNumber;
    //vector<int> recordNumOfPage, availOfPage;
    //into memory
    fileManager.openFile(path.c_str(), FileID);
    char* offset = getChars(0, 0, PAGE_SIZE);
    __RID__ = *(reinterpret_cast<int*>(offset));
    offset += sizeof(int);
    recordLength = *(reinterpret_cast<int*>(offset));
    offset += sizeof(int);
    numOfColumns = *(reinterpret_cast<int*>(offset));
    offset += sizeof(int);

    PAIR* tem = new PAIR[numOfColumns];
    for(int i = 0; i < numOfColumns; ++ i){//读出每一个表项的长度
        tem[i].second = *(reinterpret_cast<int*>(offset));
        offset += sizeof(int);
    }
    for(int i = 0; i < numOfColumns; ++ i){//读出每个表项的名称
        int temLen = *(reinterpret_cast<int*>(offset));
        offset += sizeof(int);//读出表项名称的长度
        tem[i].first = offset;
        offset += temLen;
    }
    PAIRVECTOR pairvector(tem, tem + numOfColumns);
    th.thVector = pairvector;

    recordNumber = *(reinterpret_cast<int*>(offset));
    offset += sizeof(int);
    pageNumber = *(reinterpret_cast<int*>(offset));
    offset += sizeof(int);
    for(int i = 0; i < pageNumber; ++ i){
        recordNumOfPage[i] = *(reinterpret_cast<int*>(offset));
        offset += sizeof(int);
        availOfPage[i] = *(reinterpret_cast<int*>(offset));
        offset += sizeof(int);
    }


    for(int i = 0; i < 9; ++ i)
        check[1 << i - 1] = i;

}

pair<bool,Record> Table::insertRecord(Record record){
    if(record.page>=0)
        return make_pair(false,null);
    char* buf = getChars(record.page,record.offset,recordLength);
    recordNumber++;
    int insertPage = -1;
    for(int i = 1; i < pageNumber; i++){
        if(availOfPage[i]>=0){
            insertPage = i;
            break;
        }
    }
    int recordLimit = (PAGE_SIZE<<3) / ( (recordLength<<3) + 1);
    int byteLength = (recordLimit+7) / 8;
    if(insertPage==-1){
        insertPage = pageNumber;
        pageNumber++;
        recordNumber.push_back(1);
        availOfPage.push_back(1);
        char* buffer = new char[byteLength];
        memset(buffer,0,sizeof(char)*byteLength);
        memset(buffer+byteLength-1, 0x01 , 1);
        setChars(insertPage,PAGE_SIZE-byteLength,byteLength);
        setChars(insertPage,0,buf,recordLength);
        //Record rec(this,false,insertPage,0);
        return make_pair(true, Record(this, false, insertPage, 0));
    }
    else{
        int offset = availOfPage[insertPage]*recordLength;
        setChars(insertPage,offset,buf,recordLength);
        Record rec(this,false,insertPage,offset); 

        recordNumber[insertPage]++;
        int byteIndex = (availOfPage[insertPage])/8+1;// 位数组从下标0开始
        int bitIndex = availOfPage[insertPage]%8;
        byte theByte = (1 << bitIndex);
        char* oldByte = getChars(insertPage,PAGE_SIZE-byteIndex,1);
        byte old = (byte)*oldByte;
        byte newByte = theByte | old;
        char* buffer = new char[1];
        memset(buffer,&newByte,1);
        setChars(insertPage,PAGE_SIZE-byteIndex,buffer,1);
        if(recordNumber[insertPage]==recordLimit){
            availOfPage[insertPage] = -1;
        }
        else{
            buffer = getChars(insertPage,PAGE_SIZE-byteLength,byteLength);
            int i,y;
            for(i = 0; i < byteLength; i++){
                byte x;
                memcpy(&x,buffer+byteLength-i-1,sizeof(byte));
                int yy = (x+1)&(~x)
                if(yy>0){
                    y = check[yy - 1];
                    break;
                }
            }
            availOfPage[insertPage] = i*8 + y-1;
        }
        return make_pair(true,rec);
    }
}


bool Table::updateRecord(Record real, Record dummy) {
    if(dummy.page < 0 && real.page >= 0) {
        char *res = getChars(dummy.page, dummy.offset, recordLength);
        return setChars(real.page, real.offset, res, recordLength);
    } else
        return false;
}

pair<bool,Record> Table::selectRecord(Record cond)
{
    if (cond.page >= 0) return make_pair(false, Record(this,false));
    int len = ((recordLength << 3) + 1) >> 16; // ((1<<13)=8192)*8 bits
    for (int p = 1; p < pageNumber; ++p)
        if (recordNumOfPage[p] > 0)
        {
            char *buf = getChars()
            for (int index = 0, row = 0, col = 0; index < len; ++index, ++col)
            {
                if (col == 8) 
                {
                    ++ row;
                    col = 0;
                }
            }
        }
}

#endif
// vim: ts=4
