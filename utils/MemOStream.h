#ifndef _MEM_O_STREAM_H_
#define _MEM_O_STREAM_H_

/*
 * Memory Output stream
 * 流写入二进制内存缓存。
 * 没有错误检测。
 */

class MemOStream {
private:
    char *dest, *now;
public:
    MemOStream(): dest(NULL), now(NULL) {}
    void load(char *dest)
    {
        this->dest = dest;
        this->now = dest;
    }
    void seek(size_t pos) { this->now = this->dest + pos; }
    size_t length() const { return now - dest; }

    void* putInt(const int d)
    {
        void *ret = memcpy(now, &d, sizeof(int));
        now += sizeof(int);
        return ret;
    }
    void* putString(const string s)
    {
        int len = s.size();
        void *ret = memcpy(now, s.c_str(), len + 1);
        now += len + 1;
        return ret;
    }
};

#endif
