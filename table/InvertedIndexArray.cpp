#ifndef INVERTEDINDEXARRAY_H
#define INVERTEDINDEXARRAY_H

void InvertedIndexArray::push(int offset)
{
	HardManager *hm = HardManager::getInstance();
	char* phead = hm->getChars(page, 0, PAGE_SIZE);
	int o = PAGE_SIZE - 2;
	while(*(short*)(phead + o) != -1)
	{
		o -= 2;
	}
	char buf[4];
	short s = offset;
	memcpy(buf + 2, &s, sizeof(short));
	s = -1;
	memcpy(buf, &s, sizeof(short));
	hm->setChars(page, o - 2, buf, 4);
}

int InvertedIndexArray::remove(int offset)
{
	HardManager *hm = HardManager::getInstance();
	char* phead =  hm->getChars(page, 0, PAGE_SIZE);
	int o = PAGE_SIZE - 2;
    short * p = (short*)(phead + o);
	while(*p != (short)offset && *p != -1) --p, o -= 2;
    if (-1 == *p) return -1;
	while(*p != -1)
	{
		hm->setChars(page, o, phead + o - 2, 2);
		o -= 2;
        -- p;
	}
    return 0;
}

vector<int> InvertedIndexArray::getVector()
{
	vector<int> v;
	HardManager *hm = HardManager::getInstance();
	char* phead =  hm->getChars(page, 0, PAGE_SIZE);
	int o = PAGE_SIZE - 2;
	short s = *(short*)(phead + o);
	while(s != -1)
	{
		v.push_back((int)s);
		o -= 2;
		s = *(short*)(phead + o);
	}
	return v;
}

#endif
