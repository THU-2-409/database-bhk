//
// Created by kinnplh on 2016/10/15.
//

#include "Record.h"

Record::Record(Table* _table ,int _page, int _offset) {
    page = _page;
    offset = _offset;
    table = _table;
}

/**
 * get int accord to the th
 * @return
 */

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
    return *((int*)bufRes);
}