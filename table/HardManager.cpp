#ifndef _HARD_MANAGER_CPP_
#define _HARD_MANAGER_CPP_

HardManager* HardManager::getInstance()
{
    static HardManager *instance = new HardManager();
    return instance;
}

bool HardManager::createFile(string path)
{
    return fileManager.createFile(path.c_str());
}

int HardManager::deleteFile(string path)
{
    return remove(path.c_str());
}

bool HardManager::open(string path)
{
    return fileManager.openFile(path.c_str(), fileID);
}

int HardManager::close()
{
    bufPageManager.close();
    return fileManager.closeFile(fileID);
}

bool HardManager::_preAccess(int page)
{
    if (page < 0) return false;
    if (pages.size() <= page)
    {
        int eld = pages.size() - 1;
        pages.insert(pages.end(), page - eld, NULL);
        pageIndexs.insert(pageIndexs.end(), page - eld, 0);
    }
    pages[page] = (char*) bufPageManager.getPage(fileID
            , page, pageIndexs[page]);
    return true;
}


char* HardManager::getChars(int page, int offset, int size)
{
    assert(offset + size <= PAGE_SIZE);
    assert(_preAccess(page));
    bufPageManager.access(pageIndexs[page]);
    return pages[page] + offset;
}

int HardManager::setChars(int page, int offset, const void *buf, int size)
{
    if (offset + size > PAGE_SIZE) return -1;
    if (!_preAccess(page)) return -1;
    bufPageManager.markDirty(pageIndexs[page]);
    memcpy(pages[page] + offset, buf, size);
    return 0;
}

int HardManager::getInt(int page, int offset)
{
    int *x = (int*) this->getChars(page, offset, 4);
    return *x;
}

string HardManager::getString(int page, int offset)
{
    // 字符串在文件中以\0结尾
    char *buf = this->getChars(page, offset, 1);
    return string(buf);
}

int HardManager::setInt(int page, int offset, int val)
{
    return setChars(page, offset, &val, 4);
}

int HardManager::setString(int page, int offset, string str)
{
    // \0 也要写入文件
    return setChars(page, offset, str.c_str(), str.size() + 1);
}


#endif
