#ifndef _BYTEARRAY_H_
#define _BYTEARRAY_H_

#include <assert.h>
#include <cstring>
#include <cerrno>
#include <stdio.h>

/* ByteArray
 * 字节数组封装
 */

#define BYTEARRAY_MAX_SIZE 65536

class ByteArray {
private:
    char *buf;
    int size;
    bool em;
public:
    ByteArray()
        :buf(NULL), size(0), em(false)
    {
    }

    ByteArray(const void *src, int size)
        :size(size), em(true)
    {
        //printf("cons %d %d\n",*(int*)src, size);
        assert(size > 0 && size <= BYTEARRAY_MAX_SIZE);
        buf = new char[size];
        memcpy(buf, src, size);
        //printf("cons end %d %d\n",*(int*)buf, size);
    }

    ByteArray(const ByteArray &ori)
        :size(ori.size)
    {
        buf = new char[size];
        memcpy(buf, ori.buf, size);
    }

    ByteArray & operator = (const ByteArray & src)
    {
        if (em) delete[] buf;
        em = false;
        size = src.size;
        buf = new char[size];
        memcpy(buf, src.buf, size);
        em = true;
        return *this;
    }

    ~ByteArray()
    {
        delete[] buf;
        em = false;
    }

    char& operator[] (const int i)
    {
        assert(i >= 0 && i < size);
        return buf[i];
    }

    char* c_str() 
    {
        return buf;
    }

    int getSize() const { return this->size; }

    int intCmp(ByteArray &other);
    int strCmp(ByteArray &other);
};

int ByteArray:: intCmp(ByteArray &other)
{
    if (this->getSize() != 4 && other.getSize() != 4) 
    {
        errno = 22;
        return 100;
    }
    int a = *(int*)this->buf;
    int b = *(int*)other.buf;
    if (a == b) return 0;
    return a < b ? -1 : 1;
}

int ByteArray:: strCmp(ByteArray &other)
{
    int len = this->getSize();
    int len1 = other.getSize();
    if (len > len1) len = len1;
    for (int i = 0; i < len; ++i)
    {
        if (buf[i] != other[i])
            return buf[i] < other[i] ? -1 : 1;
        if (0 == buf[i]) return 0;
    }
    errno = 42;
    return 100;
}

#endif
