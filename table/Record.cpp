//
// Created by kinnplh on 2016/10/15.
//

#include "Record.h"

Record::Record(Table* _table ,int _page, int _offset) {
    page = _page;
    offset = _offset;
    table = _table;
}

int Record::getInt(string thName) {
    TableHeader th = table->th;
    int headOffset = 0;
    for(int i = 0; i < th.getSize(); ++ i) {
        if(th[i].first.compare(thName) == 0)
            break;
        headOffset += th[i].second;
    }
    int totalOffsetInPage = headOffset + offset;
    char* bufRes = table->getChars(page, totalOffsetInPage, 4);
    return *(reinterpret_cast<int*>(bufRes));
}

string Record::getString(string thName) {
    TableHeader th = table->th;
    int headOffset = 0;
    int strLen = 0;
    for(int i = 0; i < th.getSize(); ++ i) {
        if(th[i].first.compare(thName) == 0) {
            strLen = th[i].second;
            break;
        }
        headOffset += th[i].second;
    }
    int totalOffsetInPage = headOffset + offset;
    string res  = table->getChars(page, totalOffsetInPage, strLen);
    return res;
}

bool Record::setInt(string thName, int value) {
    TableHeader th = table->th;
    int headOffset = 0;
    for (int i = 0; i < th.getSize(); ++i) {
        if (th[i].first.compare(thName) == 0)
            break;
        headOffset += th[i].second;
    }
    int totalOffsetInPage = headOffset + offset;
    return table->setChars(page, totalOffsetInPage, reinterpret_cast<char*>(&value), 4);
}

bool Record::setString(string thName, string value) {
    TableHeader th = table->th;
    int headOffset = 0;
    int strLen = 0;
    for(int i = 0; i < th.getSize(); ++ i) {
        if(th[i].first.compare(thName) == 0) {
            strLen = th[i].second;
            break;
        }
        headOffset += th[i].second;
    }
    int totalOffsetInPage = headOffset + offset;
    return table->setChars(page, totalOffsetInPage, const_cast<char*>(value.c_str()), strLen);
}