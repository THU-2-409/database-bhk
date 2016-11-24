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

struct IndexNodePointer {
    int page, offset;
    IndexNodePointer(int p,int o):page(p),offset(o){}
};

class Table {
public:
    Table():memPageTop(0),bufPageManager(&fileManager) {}
    friend class Record;
    static pair<bool, Table> createFile(TableHeader header, string path);
    static int deleteFile(string path);
    bool open(string path);
    int close();
    Record insertRecord(Record record);
    bool deleteRecord(Record record);
    bool updateRecord(Record real, Record dummy);
    pair<bool,Record> selectRecord(Record cond);

    bool createIndex(int fieldID);
    bool deleteIndex(int fieldID);
    int openIndex(int fieldID);
    bool closeIndex(int fieldID);
    Record findFromIndex(int page,Record record,int fieldID,list<IndexNodePointer>* stackRecoder);
    bool deleteFromIndex(int page,Record record,int fieldID);

    TableHeader th;
private:
    char* getChars(int page, int offset, int size);
    bool setChars(int page, int offset, char* buf, int size);
    int newMemPage();
    void releaseMemPage(int page);

    // 非接口
    bool _preAccess(int page); // true表示正常
    void _writeInfo2Hard(); // 将第一页信息写入文件系统

    void _getNewEmptyPage(int parentPage,vector<int> v,int kpLength);
    int _compare(char *a, char *b);
    void _updateNode(list<IndexNodePointer> stackRecoder);

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

    vector<pair<int,int>> indexVector;//m个（索引页序号（B树根），第几个属性）
    int emptyPageListHead;//空页链表头
    int nowIndexRootPage;

    static int check[256];
};

int Table::close()
{
    bufPageManager.close();
    return fileManager.closeFile(FileID);
}

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
    this.setChars(0, 0, buf, offset);
}

pair<bool, Table> Table::createFile(TableHeader header, string path) 
{
    Table table;
    if (!fileManager.createFile(path.c_str())) return make_pair(false, table);
    if (!table.open(path)) return make_pair(false, table);
    table.th = header;
    // 计算信息
    table.__RID__ = 0;
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
    if (!fileManager.openFile(path.c_str(), FileID)) return false;
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

    return true;
}

pair<bool,Record> Table::insertRecord(Record record){
    if(record.page>=0)
        return make_pair(false,null);
    record.setInt("#rid",__RID__++);
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
    if(dummy.page < 0 && real.page >= 0 && dummy.offset == 0) {
        char *res = getChars(dummy.page, dummy.offset, recordLength);
        return setChars(real.page, real.offset, res, recordLength);
    } else
        return false;
}

pair<bool,Record> Table::selectRecord(Record cond)
{
    if (cond.page >= 0 || cond.offset != 0) 
        return make_pair(false, Record(this,true));
    int len = ((recordLength << 3) + 1) >> 16; // ((1<<13)=8192)*8 bits
    char *ref = getChars(cond.page, cond.offset, recordLength);
    for (int p = 1; p < pageNumber; ++p)
        if (recordNumOfPage[p] > 0)
        {
            char *buf = getChars(p, 0, PAGE_SIZE);
            for (int index = 0, row = 0, col = 0; index < len; ++index, ++col)
            {
                if (col == 8) 
                {
                    ++ row;
                    col = 0;
                }
                if ((1 << col) & buf[PAGE_SIZE - row - 1])
                {
                    bool flag = true;
                    for (int i = 0, offset = index * recordLength;
                            i < th.getSize() && flag;
                            offset += th[i++].second)
                        if (th[i].first.at(0) != '#')
                        {
                            flag = memcmp(ref, buf + offset, th[i].second);
                        }
                    if (flag)
                        return make_pair(true,
                                Record(this, false, p, index * recordLength));
                }
            }
        }
    return make_pair(false, Record(this, true));
}

void Table::_getNewEmptyPage(int parentPage,vector<int> v,int kpLength){
    int offset = 0;
    int childPage = getInt(parentPage,offset+kpLength-8);
    int leafFlag = getInt(parentPage,offset+kpLength-4);
    while(childPage!=-1){
        v.push_back(childPage);
        if(leafFlag!=-1)
            _getNewEmptyPage(childPage,v,kpLength);
        offset += kpLength;
        childPage = getInt(parentPage,offset+kpLength-8);
        leafFlag = getInt(parentPage,offset+kpLength-4);
    } 
}

bool Table::createIndex(int fieldID){
    //创建索引
    indexVector.push_back(emptyPageListHead,fieldID);

    //维护空页链表头
    emptyPageListHead = getInt(emptyPageListHead,0);

    return true;
}

bool Table::deleteIndex(int fieldID){
    //删除索引
    Pair<int,int> index = indexVector.back();
    indexVector.pop_back();

    //维护空页链表
    int rootPage = index.first;
    int keySize = th[fieldID].second;//fieldID从1开始
    vector<int> newEmptyPage;
    newEmptyPage.push_back(rootPage);
    _getNewEmptyPage(rootPage,newEmptyPage,keySize+8);
    while(!newEmptyPage.empty()){
        char* buf = new char(sizeof(int));
        memcpy(buf,&emptyPageListHead,sizeof(int));
        if(!setChars(newEmptyPage.back(),0,buf,sizeof(int)))
            return false;
        emptyPageListHead = newEmptyPage.back();
        newEmptyPage.pop_back();
    }
}

int Table::openIndex(int fieldID){
    int size = indexVector.size();
    int rootPage = -1;
    for(int i = 0; i < size; i++){
        if(indexVector[i].second==fieldID){
            rootPage = indexVector[i].first;
            break;
        }
    }
    return rootPage;
}

bool Table::closeIndex(int fieldID){
    nowIndexRootPage = -1;
    return true;
}

Record Table::findFromIndex(int page,Record record,int fieldID, list<IndexNodePointer>* stackRecoder){
    string fieldName = th[fieldID].first;
    char* key = record.getString(fieldName).c_str();
    int keySize = th[fieldID].second;
    int rootPage = openIndex(fieldID);
    int leafPage = _findByKey(key,rootPage,stackRecoder,keySize);
    if(leafPage==-1)
        return Record(this,false,-1,0);
    char* dataPage = getChars(leafPage,0,PAGE_SIZE);
    int arrayIndex = 2;
    short soffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
    int offset = (int)soffset;
    while(offset!=-1){
        Record rec(this,false,leafPage,offset);
        char* recKey = getString(fieldName).c_str();
        if(_compare(key,recKey,keySize)==0)
            return rec;
        arrayIndex += 2;
        soffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
        offset = (int)soffset;
    }
    return Record(this,false,-1,0);
}

int Table::_compare(char *a, char *b,int keySize){
    if (keySize != 4)
    {
        for (int i = 0; i < keySize; ++i)
        {
            if (*a != *b) return *a < *b ? -1 : 1;
            ++a;
            ++b;
        }
    }
    else 
    {
        int i = *((int*)a), j = *((int*)b);
        if (i != j) return i < j ? -1 : 1;
    }
    return 0;
}

int Table::_findByKey(char* key, int nodePage, list<IndexNodePointer>* stackRecoder,int keySize){
    char* node = getChars(nodePage,0,PAGE_SIZE);
    int page = *(int*)(node+keySize);
    int offset = 0;
    while(page!=-1)
    {
        int ret = _compare(key,node+offset,keySize);
        if(ret==1)
        {
            offset = keySize + 8;
            page = *(int*)(node+offset+keySize);
            continue;
        }
        else if(ret==0)
        {
            stackRecoder->push_back(nodePage,offset);
            int leafFlag = *(int*)(node+offset+keySize+4);
            if(leafFlag==-1)
            {  
                return page;
            }
            else
            {
                return _findByKey(key, page, stackRecoder, keySize);    
            }
        }
        else
        {
            if(offset==0)
                break;
            offset -= keySize + 8;
            page = *(int*)(node+offset+keySize);
            stackRecoder->push_back(nodePage,offset);
            return _findByKey(key, page, stackRecoder, keySize);
        }
    }
    return -1;
}

bool Table::deleteFromIndex(int page,Record record,int fieldID){
    list<IndexNodePointer> stackRecoder;
    Record rec = findFromIndex(page,record,fieldID,&stackRecoder);
    
    //找不到需要删除的记录
    if(rec.page==-1)
        return false;

    //删除索引数据页中的记录
    char* dataPage = getChars(rec.page,0,PAGE_SIZE);
    int arrayIndex = 2;
    short dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
    short deleteOffset = (short)rec.offset;
    int count = 0;
    int deleteIndex;
    while(dataOffset!=-1){
        count++;
        if(dataOffset==deleteOffset){
            deleteIndex = arrayIndex;
        }
        arrayIndex += 2;
        dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
    }
    int arrayEnd = arrayIndex;
    int length = arrayIndex-deleteIndex;//-1位置减去记录位置
    char* buf = getChars(rec.page,PAGE_SIZE-arrayIndex);
    setChars(rec.page,PAGE_SIZE-arrayIndex+2,buf,length);

    //lazy标记数据表中相应记录为已删除 todo
    
    //维护B+树结构
    count--;
    int maxSize = (PAGE_SIZE-2)/(recordLength+2);
    if(count>=maxSize/2)
        return true;

    int parentPage = stackRecoder.back().page;
    int offset = stackRecoder.back().offset;
    char* neighborKP = getChars(parentPage, offset + keySize + 8, keySize + 8);
    int neighborPage = *(int*)(neighborKP + keySize);
    flag right = true;//判断邻居是左还是右
    if(neighborPage==-1){
        neighborKP = getChars(parentPage, offset - keySize -8, keySize + 8);
        neighborPage = *(int*)(neighborKP + keySize);
        right = false;
    }    

    char* neighbor = getChars(neighborPage,0,PAGE_SIZE);
    arrayIndex = 2;
    dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
    count = 0;
    while(dataOffset!=-1){
        count++;
        arrayIndex += 2;
        dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
    }
    if(count>maxsize/2){
        if(ritht){
            int movingDataOffset = *(short*)(neighbor+PAGE_SIZE-2);
            char* movingDataBuf = getChars(neighborPage,movingDataOffset,recordLength);
            setChars(neighborPage,PAGE_SIZE-arrayIndex+2,neighbor+PAGE_SIZE-arrayIndex,arrayIndex-2);
            setChars(page,deleteOffset,buf,recordLength);
            short copySource = -1;
            buf = new char[4];
            memcpy(buf,&copySource,sizeof(short));
            copySource = deleteOffset;
            memcpy(buf+2,&copySource,sizeof(short));
            setChars(page,PAGE_SIZE-arrayEnd,buf,4);
            Record movingRec(this,false,page,(int)deleteOffset);
            char* newK = movingRec.getString(th[fieldID].first).c_str();
            setChars(parentPage,offset,newK,th[fieldID].second);
            delete[] buf;
        }else{
            int movingDataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex+2);
            char* movingDataBuf = getChars(neighborPage,movingDataOffset,recordLength);
            setChars(neighborPage,PAGE_SIZE-arrayIndex+2,neighbor+PAGE_SIZE-arrayIndex,2);
            setChars(page,deleteOffset,buf,recordLength);
            setChars(page,PAGE_SIZE-arrayEnd,PAGE_SIZE-arrayEnd+2,arrayEnd-2);
            buf = new char[2];
            memcpy(buf,&deleteOffset,sizeof(short));
            setChars(page,PAGE_SIZE-2,buf,2);
            short neighborMaxDataOffset = *(short*)(neighborPage+PAGE_SIZE-arrayIndex+4);
            Record movingRec(this,false,neighborPage,(int)neighborMaxDataOffset);
            char* newK = movingRec.getString(th[fieldID].first).c_str();
            setChars(parentPage,offset-th[fieldID].second-8,newK,th[fieldID].second);
            delete[] buf;
        }
    }else{
        buf = new char[PAGE_SIZE];
        if(right){
            short bufOffset = 0;
            arrayIndex = 2;
            dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
            while(dataOffset!=-1){
                memcpy(buf+bufOffset,dataPage+dataOffset,recordLength);
                memcpy(buf+PAGE_SIZE-arrayIndex,&bufOffset,sizeof(short));
                arrayIndex += 2;
                bufOffset += recordLength
                dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
            }
            tmp = arrayIndex-2;
            arrayIndex = 2;
            dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
            while(dataOffset!=-1){
                memcpy(buf+bufOffset,neighbor+dataOffset,recordLength);
                memcpy(buf+PAGE_SIZE-tmp-arrayIndex,&bufOffset,sizeof(short));
                arrayIndex += 2;
                bufOffset += recordLength
                dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
            }
            setChars(page,0,buf,PAGE_SIZE);
            delete[] buf;
            buf = new char[4];
            memcpy(buf,&emptyPageListHead,sizeof(int));
            setChars(neighborPage,0,buf,sizeof(int));
            emptyPageListHead = neighborPage;
            int keySize = th[fieldID].second;
            setChars(parentPage,offset,parentPage+offset+keySize+8,keySize);
            setChars(parentPage,offset+keySize+8,parentPage+offset+2*(keySize+8),PAGE_SIZE-(offset+2*(keySize+8)));
            int nodeOffset = 0;
            int nodePage = *(int*)(parentPage+nodeOffset+keySize);
            int nodeCount = 0;
            while(nodePage!=-1){
                nodeCount ++;
                nodeOffset += keySize + 8;
                nodePage = *(int*)(parentPage+nodeOffset+keySize);
            }
            int nodeMaxSize = (int)(PAGE_SIZE/(keySize + 8)) - 1;
            if(nodeCount < nodeMaxSize/2)
                _updateNode(stackRecoder,keySize);
            delete[] buf;
        }else{
            short bufOffset = 0;
            arrayIndex = 2;
            dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
            while(dataOffset!=-1){
                memcpy(buf+bufOffset,neighbor+dataOffset,recordLength);
                memcpy(buf+PAGE_SIZE-arrayIndex,&bufOffset,sizeof(short));
                arrayIndex += 2;
                bufOffset += recordLength
                dataOffset = *(short*)(neighbor+PAGE_SIZE-arrayIndex);
            }
            tmp = arrayIndex-2;
            arrayIndex = 2;
            dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
            while(dataOffset!=-1){
                memcpy(buf+bufOffset,dataPage+dataOffset,recordLength);
                memcpy(buf+PAGE_SIZE-tmp-arrayIndex,&bufOffset,sizeof(short));
                arrayIndex += 2;
                bufOffset += recordLength
                dataOffset = *(short*)(dataPage+PAGE_SIZE-arrayIndex);
            }
            setChars(neighborPage,0,buf,PAGE_SIZE);
            delete[] buf;
            buf = new char[4];
            memcpy(buf,&emptyPageListHead,sizeof(int));
            setChars(page,0,buf,sizeof(int));
            emptyPageListHead = page;
            int keySize = th[fieldID].second;
            setChars(parentPage,offset-keySize-8,parentPage+offset,PAGE_SIZE-offset);
            int nodeOffset = 0;
            int nodePage = *(int*)(parentPage+nodeOffset+keySize);
            int nodeCount = 0;
            while(nodePage!=-1){
                nodeCount ++;
                nodeOffset += keySize + 8;
                nodePage = *(int*)(parentPage+nodeOffset+keySize);
            }
            int nodeMaxSize = (int)(PAGE_SIZE/(keySize + 8)) - 1;
            if(nodeCount < nodeMaxSize/2)
                _updateNode(stackRecoder,keySize);
            delete[] buf;
        }
    }
    return true;
}

void Table::_updateNode(list<IndexNodePointer> stackRecoder,int fieldID){
    int childPage = stackRecoder.back().page;
    int childOffset = stackRecoder.back().offset;
    stackRecoder.pop_back();

    if(stackRecoder.empty()==stackRecoder.end())
        return;

    int parentPage = stackRecoder.back().page;
    int parentOffset = stackRecoder.back().offset;
    char* parent = getChars(parentPage,parentOffset,PAGE_SIZE);

    int keySize = th[fieldID].second;
    int kpLength = keySize + 8;

    int childNeighborPage = *(int*)(parent+parentOffset+kpLength+keySize);
    bool right = true;
    if(childNeighborPage==-1){
        childNeighborPage = *(int*)(parent+parentOffset-8);
        right = false;
    }

    int childNeighborNodeCount = 0;
    int childNeighborOffset = 0;
    char* childNeighbor = getChars(childNeighborPage,0,PAGE_SIZE);
    int tmpPage = *(int*)(childNeighbor+childNeighborOffset+keySize);
    while(tmpPage!=-1){
        childNeighborNodeCount++;
        childNeighborOffset += kpLength;
        tmpPage = *(int*)(childNeighbor+childNeighborOffset+keySize);
    }
    int maxNode = (int)(PAGE_SIZE/kpLength) - 1;

    if(childNeighborNodeCount > maxNode/2){
        if(right){
            char* movingKPBuf = getChars(childNeighborPage,0,kpLength);

            char* child = getChars(childPage,0,PAGE_SIZE);
            int childKPendOffset = 0;
            int tmpPage = *(int*)(child+childKPendOffset+keySize);
            while(tmpPage!=-1){
                childKPendOffset += kpLength;
                tmpPage = *(int*)(child+childKPendOffset+keySize);
            }
            setChars(childPage,childKPendOffset+kpLength,childPage+childKPendOffset,kpLength);
            setChars(childPage,childKPendOffset,movingKPBuf,kpLength);

            setChars(childNeighborPage,0,childNeighbor+kpLength,PAGE_SIZE-kpLength);

            setChars(parentPage,parentOffset,childPage+childKPendOffset,keySize);
        }else{
            char* movingKPBuf = getChars(childNeighborPage,childNeighborOffset-kpLength,kpLength);

            setChars(childPage,kpLength,child,PAGE_SIZE-kpLength);
            setChars(childPage,0,movingKPBuf,kpLength);

            setChars(childNeighborPage,childNeighborOffset-kpLength,childNeighbor+childNeighborOffset,kpLength);

            setChars(parentPage,parentOffset-kpLength,childNeighbor+childNeighborOffset-kpLength*2,keySize);
        }
    }else{
        if(right){
            char* child = getChars(childPage,0,PAGE_SIZE);
            int childKPendOffset = 0;
            int tmpPage = *(int*)(child+childKPendOffset+keySize);
            while(tmpPage!=-1){
                childKPendOffset += kpLength;
                tmpPage = *(int*)(child+childKPendOffset+keySize);
            }
            setChars(childPage,childKPendOffset,neighbor,childNeighborOffset+kpLength);

            buf = new char[4];
            memcpy(buf,&emptyPageListHead,sizeof(int));
            setChars(childNeighborPage,0,buf,sizeof(int));
            delete[] buf;
            emptyPageListHead = childNeighborPage;

            setChars(parentPage,parentOffset,parent+parentOffset+kpLength,PAGE_SIZE-parentOffset-kpLength);
            int nodeOffset = 0;
            int nodePage = *(int*)(parentPage+nodeOffset+keySize);
            int nodeCount = 0;
            while(nodePage!=-1){
                nodeCount ++;
                nodeOffset += keySize + 8;
                nodePage = *(int*)(parentPage+nodeOffset+keySize);
            }
            int nodeMaxSize = (int)(PAGE_SIZE/kpLength) - 1;
            if(nodeCount < nodeMaxSize/2)
                _updateNode(stackRecoder,keySize);
        }else{
            setChars(childNeighborPage,childNeighborOffset,child,PAGE_SIZE-childNeighborOffset);

            buf = new char[4];
            memcpy(buf,&emptyPageListHead,sizeof(int));
            setChars(childPage,0,buf,sizeof(int));
            delete[] buf;
            emptyPageListHead = childPage;

            setChars(parentPage,parentOffset-kpLength,parent+parentOffset,PAGE_SIZE-parentOffset);
            int nodeOffset = 0;
            int nodePage = *(int*)(parentPage+nodeOffset+keySize);
            int nodeCount = 0;
            while(nodePage!=-1){
                nodeCount ++;
                nodeOffset += keySize + 8;
                nodePage = *(int*)(parentPage+nodeOffset+keySize);
            }
            int nodeMaxSize = (int)(PAGE_SIZE/kpLength) - 1;
            if(nodeCount < nodeMaxSize/2)
                _updateNode(stackRecoder,keySize);
        }
    }     
}


#endif
// vim: ts=4
