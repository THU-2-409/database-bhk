#ifndef RECORD_H
#define RECORD_H

bool Record::next()
{
	if(oi >= v.size() - 1)
		return false;

	oi ++;
	offset = v[oi];
	return true;
}

RecordData Record::getData()
{
	RecordData rd;
	int o = offset;
	HardManager *hm = HardManager::getInstance();
	char* rec = hm->getChars(page, o, info->recordLen); //record
	o += 4;
	char* nullArray = hm->getChars(page, o, info->nullLength);
	o += info->nullLength;
	int cols = info->header.getColNums();
	for(int i = 0; i < cols; i++)
	{
		int naIndex = i / 8; //null array index
		char c = *(nullArray + naIndex);
		bool b = c & (1 << (i % 8));
		if(b)
		{
			rd.setNULL(info->header.getName(i));
		}
		else
		{
			if(info->header.getType(i) == COL_TYPE_VSTR) 
			{
				string str(rec + o, info->header.getSize(i));
				rd.setString(info->header.getName(i), str);
			}
			else
			{
				ByteArray ba(rec + o, info->header.getSize(i));
				rd.setBA(info->header.getName(i), ba);	
			}
		}
		o += info->header.getSize(i);
	}
	return rd;
}

void Record::setData(RecordData rd)
{
	char buf[info->recordLen];
	int o = 0;
	*(int*)buf = info->RID;
	o += 4;
	char* nullArray = buf + o;
	o += info->nullLength;
	int cols = info->header.getColNums();
	char c;
	for(int i = 0; i < cols; i++)
	{
		if((i % 8) == 0)
		{
			c = 0;
		}
		pair<bool, ByteArray> rst = rd.getBA(info->header.getName(i));
		if(rst.first == false)
		{
			c = c | (1 << (i % 8));
		}
		else
		{
			memset(buf + o, 0, info->header.getSize(i));
			int len = min(info->header.getSize(i), rst.second.getSize());
			memcpy(buf + o, rst.second.c_str(), len);
		}
		if((i % 8) == 7 || i == cols - 1)
		{
			*(nullArray + i / 8) = c;
		}
		o += info->header.getSize(i);
	}
	HardManager *hm = HardManager::getInstance();
	hm->setChars(page, offset, buf, info->recordLen);
}

#endif