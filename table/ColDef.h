#ifndef _COLDEF_H_
#define _COLDEF_H_

#include <string>
using namespace std;

#define COL_REG_T 0
#define COL_NOT_NULL_T 1
#define COL_KEY_T 2

class ColDef
{
    public:
        string name;
        int size;
        int type;
};

#endif
