//
// Created by kinnplh on 2016/10/15.
//

#include "Record.h"

Record::Record(Table* _table ,int _page, int _offset) {
    page = _page;
    offset = _offset;
    table = _table;
}


int Record::getInt(string) {

}