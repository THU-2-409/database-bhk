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

    void insert(char *value, int page, int offset);
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

    void _insertDeep(list<IndexNodePointer> &stack, char *value, int page, int offset);
    void _repairToTop(list<IndexNodePointer> &stack);
    void _insert2NodeArray(IndexNodePointer r, char *val)
    IndexNodePointer _findNodeArray4P(IndexNodePointer r, IndexNodePointer val)
    IndexNodePointer _allocBNode();
    int _compare(char *a, char *b);
};

int Index:: _compare(char *a, char *b)
{
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

void Index:: insert(char *value, int page, int offset)
{
    list<IndexNodePointer> stack;
    IndexNodePointer p;
    p.page = 1;
    p.offset = 0;
    stack.push_back(p);
    _insertDeep(stack, value, page, offset);
}

void Index:: _insert2NodeArray(IndexNodePointer r, char *val)
{
    char *tmp = new char[keySize + 8];
    memcpy(tmp, val, keySize + 8);
    char *leave = new char[nodeSize];
    char *pleave = this.getChars(r.page, r.offset, nodeSize);
    int l_offset = 0;
    for (int i = 0; i < B_TREE_NODE_SIZE; ++i)
    {
        int *pPage = (int*)(pleave + keySize);
        if (*pPage == -1)
        {
            // 数组的结束
            memcpy(leave + l_offset, tmp, keySize + 8);
            l_offset += keySize + 8;
            memset(leave + l_offset + keySize, -1, sizeof(int));
            l_offset += keySize + 8;
            break;
        }
        int cmp = _compare(tmp, pleave);
        if (cmp < 0)
        {
            memcpy(leave + l_offset, tmp, keySize + 8);
            l_offset += keySize + 8;
            memcpy(tmp, pleave, keySize + 8);
        }
        else 
        {
            memcpy(leave + l_offset, pleave, keySize + 8);
            l_offset += keySize + 8;
        }
        pleave += keySize + 8;
    }
    this.setChars(r.page, r.offset, leave, l_offset);
}

void Index:: _insertDeep(list<IndexNodePointer> &stack, char *value, int page, int offset)
{
    IndexNodePointer r = stack.back();
    if (stack.size() > this.treeDepth)
    {
        // 叶结点
        char *tmp = new char[keySize + 8];
        memcpy(tmp, value, keySize);
        memcpy(tmp + keySize, &page, sizoef(int));
        memcpy(tmp + keySize + 4, &offset, sizoef(int));
        _insert2NodeArray(r, tmp);
        _repairToTop(stack);
    }
    else
    {
        // 找到刚好大于左边的位置
        char *pleave = this.getChars(r.page, r.offset, nodeSize);
        for (int i = 0; i < B_TREE_NODE_SIZE; ++i)
        {
            int *pPage = (int*)(pleave + keySize * 2 + 8);
            if (*pPage == -1)
            {
                IndexNodePointer p;
                pPage = (int*)(pleave + keySize);
                p.page = pPage[0];
                p.offset = pPage[1];
                stack.push(p);
                _insertDeep(stack, value, page, offset);
                return ;
            }
            int cmp = _compare(value, pleave + keySize + 8);
            if (cmp >= 0)
            {
                IndexNodePointer p;
                pPage = (int*)(pleave + keySize);
                p.page = pPage[0];
                p.offset = pPage[1];
                stack.push(p);
                _insertDeep(stack, value, page, offset);
                return ;
            }
        }
    }
}

IndexNodePointer Index:: _findNodeArray4P(IndexNodePointer r, IndexNodePointer val)
{
    char *pnode = getChars(r.page, r.offset, nodeSize);
    for (int i = 0; i < B_TREE_NODE_SIZE; ++i)
    {
        int *pPage = (int*)(pnode + keySize);
        int *pOfs = (int*)(pnode + keySize + 4);
        if (*pPage == -1) break;
        if (*pPage == val.page && *pOfs == val.offset) return pnode;
        pnode += keySize + 8;
    }
    r.page = -1;
    return r;
}

void Index:: _repairToTop(list<IndexNodePointer> stack)
{
    for (; ;)
    {
        if (r.size <= 0) break;
        IndexNodePointer r = stack.back();
        stack.pop_back();
        // 是否爆满
        int flag = true;
        char *pnode = this.getChars(r.page, r.offset, nodeSize);
        for (int i = 0; i < B_TREE_NODE_SIZE; ++i)
        {
            int *pPage = (int*)(pnode + keySize);
            if (*pPage == -1)
            {
                flag = false;
                break;
            }
        }
        if (!flag) return ;
        // 分裂
        IndexNodePointer p1 = _allocBNode();
        IndexNodePointer p2 = _allocBNode();
        char *pnode = this.getChars(r.page, r.offset, nodeSize);
        int ofs1 = 0, ofs2 = 0;
        IndexNodePointer tmpNP;
        memcpy(&tmpNP, pnode + keySize, sizeof(IndexNodePointer));
        for (int i = 0; i < B_TREE_NODE_SIZE; ++i)
        {
            if (i + i < B_TREE_NODE_SIZE)
            {
                setChars(p1.page, p1.offset + ofs1, pnode, keySize + 8);
                ofs1 += keySize + 8;
            }
            else
            {
                setChars(p2.page, p2.offset + ofs2, pnode, keySize + 8);
                ofs2 += keySize + 8;
            }
            pnode += keySize + 8;
        }
        int tmp = -1;
        setChars(p1.page, p1.offset + ofs1 + keySize, &tmp, sizeof(int));
        setChars(p2.page, p2.offset + ofs2 + keySize, &tmp, sizeof(int));
        // 修改父亲
        if (stack.size() > 0)
        {
            // 现在不是根
            IndexNodePointer pp = _findNodeArray4P(stack.back(), tmpNP);
            setChars(pp.page, pp.offset + keySize, &p1, sizeof(IndexNodePointer));
            _insert2NodeArray(stack.back(), getChars(p2.page, p2.offset, nodeSize));
        }
        else 
        {
            // 分裂根结点
            int offset = 0;
            setChars(1, offset, getChars(p1.page, p1.offset, nodeSize), keySize + 8);
            offset += keySize + 8;
            setChars(1, offset, getChars(p2.page, p2.offset, nodeSize), keySize + 8);
            offset += keySize + 8;
            setChars(1, offset + keySize, &tmp, sizeof(int));
            return ;
        }
    }
}

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
    // 创建B树根节点
    int page = -1;
    Index.setChars(1, keySize, &page, sizeof(int));
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
