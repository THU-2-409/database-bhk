#ifndef _VALUE_H_
#define _VALUE_H_

#include <string>
using namespace std;

#define VAL_INT 10
#define VAL_STRING 11
#define VAL_NULL 12

class Value
{
    public:
    	int type;
 		int val;
        string str;       
};

#endif