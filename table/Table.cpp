#ifndef _TABLE_CPP_
#define _TABLE_CPP_

void Table:: insert(RecordData data)
{
    Record dst = this->trash.allocRecord();
    dst.setData(data);
}

vector<Record> Table:: find(RecordData data);

#endif
