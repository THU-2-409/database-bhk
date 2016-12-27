#ifndef _TRASH_CPP_
#define _TRASH_CPP_

Trash:: Trash(Table * table)
    :table(table)
{}

Record Trash::allocRecord()
{
    int &nextNewRecPage = table->info.nextNewRecPage;
    int &nextNewRecOffset = table->info.nextNewRecOffset;
    vector<int> v;
    if(nextNewRecPage == -1)
        allocPage();
    Record ret(nextNewRecPage, nextNewRecOffset, &(table->info), v);
    HardManager *hm = HardManager::getInstance();
    char* rec = hm->getChars(nextNewRecPage, nextNewRecOffset, table->info.recordLen);
    nextNewRecPage = *(int*)rec;
    nextNewRecOffset = *(short*)(rec + 4);
    return ret;
}

void Trash::freeRecord(int page, int offset)
{
    int &nextNewRecPage = table->info.nextNewRecPage;
    int &nextNewRecOffset = table->info.nextNewRecOffset;
    HardManager *hm = HardManager::getInstance();
    char buf[6];
    *(int*)buf = nextNewRecPage;
    *(short*)(buf + 4) = (short)nextNewRecOffset;
    hm->setChars(page, offset, buf, 6);
    nextNewRecPage = page;
    nextNewRecOffset = offset;
}

DataPage Trash:: allocPage()
{
    int &maxPageID = table->info.maxPageID;
    int &nextNewPage = table->info.nextNewPage;
    int ret = nextNewPage;
    HardManager * h = HardManager::getInstance();
    if (nextNewPage)
    {
        nextNewPage = h->getInt(nextNewPage, 0);
    }
    else
    {
        ret = ++ maxPageID;
        // 将空位置放入rec的链表
        int unit = table->info.recordLen;
        int size = table->info.dataPageRoom;
        int offset = 4;
        for (int i = 1; i < size; ++i, offset += unit)
        {
            h->setInt(ret, offset, ret);
            short s = offset + unit;
            h->setChars(ret, offset + 4, &s, 2);
        }
        int &nextPage = table->info.nextNewRecPage;
        int &nextOffset = table->info.nextNewRecOffset;
        h->setInt(ret, offset, nextPage);
        short tmp = nextOffset;
        h->setChars(ret, offset + 4, &tmp, 2);
        tmp = -1;
        h->setChars(ret, PAGE_SIZE - 2, &tmp, 2);
        // rec链表头维护
        nextPage = ret;
        nextOffset = 4;
    }
    // 将此页放入“占用”的数据页链表
    int &dataPageHead = table->info.dataPageHead;
    h->setInt(ret, 0, dataPageHead);
    dataPageHead = ret;
    return DataPage(ret, table);
}

void Trash:: freePage(int page)
{
    HardManager * h = HardManager::getInstance();
    int &nextPage = table->info.nextNewPage;
    h->setInt(page, 0, nextPage);
    nextPage = page;
}

#endif
