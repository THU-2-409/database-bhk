#ifndef _TABLEHEADER_CPP_
#define _TABLEHEADER_CPP_

ByteArray TableHeader:: dump()
{
    char buf[PAGE_SIZE];
    MemOStream os;
    os.load(buf);
    os.putString(schema);
    os.putInt(defs.size());
    for (int i = 0; i < defs.size(); ++i)
    {
        os.putString(getName(i));
        os.putInt(getSize(i));
        os.putInt(getType(i));
    }
    // 写完
    ByteArray res(buf, os.length());
    return res;
}

int TableHeader:: load(char *buf)
{
    MemIStream is;
    is.load(buf);
    is.getString(schema);
    int colNums;
    is.getInt(colNums);
    defs.clear();
    defs.insert(defs.end(), colNums, ColDef());
    dict.clear();
    for (int i = 0; i < colNums; ++i)
    {
        is.getString(defs[i].name);
        is.getInt(defs[i].size);
        is.getInt(defs[i].type);
        dict[defs[i].name] = i;
    }
    return is.length();
}


#endif
