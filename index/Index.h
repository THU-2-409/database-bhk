#ifndef INDEX_H
#define INDEX_H

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
#define B_TREE_NODE_SIZE 100
using namespace std;

struct IndexNodePointer {
    int page, offset;
};

class Index {
public:
    Index(): bufPageManager(&fileManager) {}
    static pair<bool, Index> createFile(string table, string key, int keySize, string path);
    static int deleteFile(string path);
    bool open(string path);
    int close();
private:
    // Index的信息
    int keySize, treeDepth, nodeSize;
    IndexNodePointer emptyListHead;
    FileManager fileManager;
    BufPageManager bufPageManager;
    // runtime
    int FileID;

    void _writeInfo2Hard();
    char* getChars(int page, int offset, int size);
    bool setChars(int page, int offset, char *buf, int size);
};

int Index:: close() 
{
    bufPageManager.close();
    return fileManager.closeFile(FileID);
}

// table: 索引的表； key：属性名; 
pair<bool, Index> Index::createFile(Table table, string key, int keySize, string path) 
{
    Index index;
    if (!fileManager.createFile(path.c_str())) return make_pair(false, table);
    if (!Index.open(path)) return make_pair(false, table);
    table.createIndex(key, path);
    Index.table = table;
    Index.keySize = keySize;
    Index.treeDepth = 0;
    Index.emptyListHead.page = 1;
    Index.emptyListHead.offset = 0;
    // 计算
    Index.nodeSize = (keySize + 8) * B_TREE_NODE_SIZE;
    // 写入
    Index._writeInfo2Hard();
}

void Index:: _writeInfo2Hard()
{
    char *buf = new char[PAGE_SIZE];
    int offset = 0;
    memcpy(buf + offset, &keySize, sizeof(int)); // 属性的类型占用的空间
    offset += sizeof(int);
    memcpy(buf + offset, &treeDepth, sizeof(int)); // B树的深度
    offset += sizeof(int);
    memcpy(buf + offset, &nodeSize, sizeof(int)); // 一个B树结点的大小
    offset += sizeof(int);
    memcpy(buf + offset, &emptyListHead, sizeof(IndexNodePointer)); // 空位链表头
    offset += sizeof(IndexNodePointer);

    // 写入
    this.setChars(0, 0, buf, offset);
}

bool Index:: setChars(int page, int offset, char *buf, int size)
{
    if (offset + size > PAGE_SIZE) return false;
    char *ref;
    int tmp;
    ref = bufPageManger.getPage(FileID, page, tmp);
    bufPageManager.markDirty(tmp);
    memcpy(ref + offset, buf, size);
    return true;
}

char* Index:: getChars(int page, int offset, int size)
{
    if (offset + size > PAGE_SIZE) return NULL;
    char *ref;
    int tmp;
    ref = bufPageManger.getPage(FileID, page, tmp);
    bufPageManger.access(tmp);
    return ref;
}

int Index:: deleteFile(string path) {
    return remove(path);
}

bool Index:: open(string path)
{
    if (!fileManger.openFile(path)) return false;

    // 读入第一页信息
    int offset = 0;
    char *buf = this.getChars(0, 0, PAGE_SIZE);
    memcpy(&keySize, buf + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&treeDepth, buf + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&nodeSize, buf + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&emptyListHead, buf + offset, sizeof(emptyListHead));
    offset += sizeof(emptyListHead);

    return true;
}


#endif
