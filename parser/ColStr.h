#ifndef _H_COL_STR_
#define _H_COL_STR_

#include <utility>

typedef std::pair<string, string> ColStr;

ColStr genColStr(const char * tb, const char * st)
{
    string a(tb);
    string b(st);
    return ColStr(a, b);
}

#endif
