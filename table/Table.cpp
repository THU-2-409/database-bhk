#ifndef _TABLE_CPP_
#define _TABLE_CPP_

void Table:: insert(RecordData data);

vector<Record> Table::find(RecordData data)
{
	vector<Record> v;

	DataPage dp(info.dataPageHead, this);
	while(dp.getPageID() != 0)
	{
		Record rec = dp.first();
		RecordData rd = rec.getData();
		map< string, pair<bool, ByteArray> >::iterator it;
		bool flag = true;
		for(it = data.begin(); it != data.end(); it++)
		{
			pair<bool, ByteArray> rdata = rd.getBA(it->first);
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
			v.push_back(rec);
		}
		dp = dp.next();
	}

	return v;
}

#endif
