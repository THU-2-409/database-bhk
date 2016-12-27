#ifndef _TABLEINFO_CPP_
#define _TABLEINFO_CPP_

TableInfo:: TableInfo()
    : header("", vector<ColDef>()),
    RID(0), nextNewPage(0), nextNewRecPage(-1),
    nextNewRecOffset(0), maxPageID(0),
    dataPageHead(0)
{
}

void TableInfo:: loadFrom()
{
    HardManager *hard = HardManager::getInstance();
    char *buf = hard->getChars(0, 0, PAGE_SIZE);
    this->load(buf);
    this->make();
}

bool TableInfo:: writeBack()
{
    HardManager *hard = HardManager::getInstance();
    ByteArray bufa = this->dump();
    return hard->setChars(0, 0, bufa.c_str(), bufa.getSize());
}

ByteArray TableInfo:: dump()
{
    ByteArray buf0 = header.dump();
    char buf[PAGE_SIZE];
    memcpy(buf, buf0.c_str(), buf0.getSize());
    MemOStream os;
    os.load(buf);
    os.seek(buf0.getSize());
    // 开始写
    os.putInt(RID);
    os.putInt(nextNewPage);
    os.putInt(nextNewRecPage);
    os.putInt(nextNewRecOffset);
    os.putInt(maxPageID);
    os.putInt(dataPageHead);
    // 写完
    //printf("dump_len:%lu - %d\n", os.length(), PAGE_SIZE);
    ByteArray res(buf, os.length());
    return res;
}

int TableInfo:: load(char *buf)
{
    int offset = header.load(buf);
    MemIStream is;
    is.load(buf + offset);
    // 开始读
    is.getInt(RID);
    is.getInt(nextNewPage);
    is.getInt(nextNewRecPage);
    is.getInt(nextNewRecOffset);
    is.getInt(maxPageID);
    is.getInt(dataPageHead);
    // 读完
    //printf("load_len:%lu - %d\n", is.length() + offset, PAGE_SIZE);
    return offset + is.length();
}

void TableInfo:: make()
{
    int cnt = header.getColNums();
    int tmp = ((cnt - 1) / 8) + 1;
    nullLength = tmp;
    int res = 4 + tmp; // __RID__ + 倒排数组
    for (int i = 0; i < cnt; ++i)
        res += header.getSize(i);
    if (res < 6) res = 6;
    recordLen = res;

    int ret = PAGE_SIZE - 4;
    dataPageRoom = (ret - 2) / (res + 2);

    pk_col = -1;
    for (int i = 0; i < cnt; ++i)
        if (header.getConstraint(i) == COL_KEY_T)
        {
            pk_col = i;
            break;
        }
}

#endif
