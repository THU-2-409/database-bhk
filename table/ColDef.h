#ifndef _COLDEF_H_
#define _COLDEF_H_

#include <string>
using namespace std;

#define COL_REG_T 0
#define COL_NOT_NULL_T 1
#define COL_KEY_T 2

#define COL_TYPE_VINT 0
#define COL_TYPE_VSTR 1

class ColDef
{
    public:
        string name;
        int size;
        int type;
        int constraint;
};

#endif
