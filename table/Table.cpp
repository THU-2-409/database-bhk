#ifndef _TABLE_CPP_
#define _TABLE_CPP_

vector<Record> Table::find(RecordData data)
{
	vector<Record> v;

	DataPage dp(info.dataPageHead, this);
	while(dp.getPageID() != 0)
	{
		Record rec = dp.first();
		while(true)
		{
			//printf("rec page: %d, offset: %d\n", rec.getPageID(), rec.getOffset());
			RecordData rd = rec.getData();
			map< string, pair<bool, ByteArray> >::iterator it;
			bool flag = true;
			for(it = data.begin(); it != data.end(); it++)
			{
				//printf("col name: %s\n", it->first.c_str());
				pair<bool, ByteArray> rdata = rd.getBA(it->first);
				//printf("in find for loop: %d\n", *(int*)rdata.second.c_str());
				if(rdata.first != it->second.first)
				{
					flag = false;
					break;
				}
				if(!rdata.first) continue;
				int type = info.header.getType(it->first);
				if(type == COL_TYPE_VINT)
				{
					if(rdata.second.intCmp(it->second.second))
					{
						flag = false;
						break;
					}
				}
				else
				{
					if(rdata.second.strCmp(it->second.second))
					{
						flag = false;
						break;
					}
				}
			}
			if(flag)
			{
                Record tmp(rec.getPageID(), rec.getOffset(),
                        &info, vector<int>());
				v.push_back(tmp);
			}
			if(rec.next() == false)
				break;
		}
		dp = dp.next();
	}

	return v;
}

void Table:: insert(RecordData data)
{
    ++ info.RID;
    Record dst = this->trash.allocRecord();
    dst.setData(data, true);
    InvertedIndexArray inv(dst.getPageID());
    inv.push(dst.getOffset());
}


#endif
