#ifndef _MEM_I_STREAM_H_
#define _MEM_I_STREAM_H_

/*
 * Memory Input stream
 * 流读取内存二进制缓存中的值。
 * 没有错误检测。
 */

class MemIStream {
private:
    char *src, *now;
public:
    MemIStream(): src(NULL), now(NULL) {}
    void load(char *src) 
    {
        this->src = src; 
        this->now = src;
    }
    void seek(size_t pos) { this->now = this->src + pos; }
    size_t length() const { return now - src; }

    void* getInt(int &d)
    {
        memcpy(&d, now, sizeof(int));
        now += sizeof(int);
        return &d;
    }
    void* getString(string &s)
    {
        int len = strlen(now);
        s = string(now);
        now += len + 1;
        return &s;
    }
};

#endif
