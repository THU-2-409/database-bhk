#ifndef RECORDDATA_H
#define RECORDDATA_H

pair<bool, int> RecordData::getInt(string fname)
{
	pair<bool, ByteArray> rst = getBA(fname);
	return make_pair(rst.first, *(int*)(rst.second.c_str()));
}

void RecordData::setInt(string fname, int value)
{
	//printf("in setInt%d\n", value);
	//m[fname] = make_pair(true, ByteArray(&value, 4));
	m.insert(make_pair(fname, make_pair(true, ByteArray(&value, 4))));
	//printf("after set%d\n", *(int*)(m[fname].second.c_str()));
}

pair<bool, string> RecordData::getString(string fname)
{
	pair<bool, ByteArray> rst = getBA(fname);
	return make_pair(rst.first, string(rst.second.c_str()));
}

void RecordData::setString(string fname, string value)
{
	//m[fname] = make_pair(true, ByteArray(value.c_str(), value.size() + 1));
	m.insert(make_pair(fname, make_pair(true, ByteArray(value.c_str(), value.size() + 1))));
}

bool RecordData:: isNULL(string name)
{
    return !m[name].first;
}

void RecordData::setNULL(string fname)
{
	//m[fname] = make_pair(false, ByteArray());
	m.insert(make_pair(fname, make_pair(false, ByteArray())));
}

pair<bool, ByteArray> RecordData::getBA(string fname)
{
	return m[fname];
}

void RecordData::setBA(string fname, ByteArray value)
{
	//m[fname] = make_pair(true, value);
	m.insert(make_pair(fname, make_pair(true, value)));
}

#endif
