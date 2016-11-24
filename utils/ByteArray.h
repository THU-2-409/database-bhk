#ifndef _BYTEARRAY_H_
#define _BYTEARRAY_H_

#include <assert.h>
#include <cstring>

/* ByteArray
 * 字节数组封装
 */

#define BYTEARRAY_MAX_SIZE 65536

class ByteArray {
private:
    char *buf;
    int size;
public:
    ByteArray(const void *src, int size)
        :size(size)
    {
        assert(size > 0 && size <= BYTEARRAY_MAX_SIZE);
        buf = new char[size];
        memcpy(buf, src, size);
    }

    ~ByteArray()
    {
        delete[] buf;
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
};

#endif
