#ifndef _DATAPAGE_CPP_
#define _DATAPAGE_CPP_

DataPage:: ~DataPage()
{
    if (NULL != invArr)
        delete invArr;
}

DataPage:: DataPage(int page, Table * table)
    :   page(page), table(table), invArr(NULL)
{
}

Record DataPage:: getRecord(int index)
{
    int offset = 4 + index * table->info.recordLen;
    return Record(page, offset, &(table->info), vector<int>());
}

DataPage DataPage:: next()
{
    HardManager * h = HardManager::getInstance();
    int nx = h->getInt(page, 0);
    return DataPage(nx, table);
}

Record DataPage:: first()
{
    if (NULL == invArr)
    {
        invArr = new InvertedIndexArray(page);
    }
    vector<int> vec = invArr.getVector();
    if (vec.size())
        return Record(page, vec.front(), &(table->info), vec);
    else
        return this->getRecord(0);
}

void DataPage:: removeRecord(int offset)
{
    if (NULL == invArr)
    {
        invArr = new InvertedIndexArray(page);
    }
    if (invArr->remove(offset)) return ;
    int &nextNewRecPage = table->info.nextNewRecPage;
    int &nextNewRecOffset = table->info.nextNewRecOffset;
    HardManager * h = HardManager::getInstance();
    h->setInt(page, offset, nextNewRecPage);
    short tmp = nextNewRecOffset;
    h->setChars(page, offset, &tmp, sizeof(short));
    nextNewRecPage = page;
    nextNewRecOffset = offset;
    short * ps = (short*)h->getChars(page, PAGE_SIZE - 2, 2);
    if (-1 == *ps) table->trash.freePage(page);
}

#endif
