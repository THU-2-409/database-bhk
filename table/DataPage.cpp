#ifndef _DATAPAGE_CPP_
#define _DATAPAGE_CPP_

DataPage:: DataPage(int page, Table * table)
    :   page(page), table(table)
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
    // TODO
}

#endif
