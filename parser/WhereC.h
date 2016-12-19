#ifndef _WHEREC_H_
#define _WHEREC_H_

#include <string>
#include "Value.h"
using namespace std;

#define WC_EQU 20
#define WC_NOT_EQU 21
#define WC_LEQ 22
#define WC_GEQ 23
#define WC_LE 24
#define WC_GR 25
#define WC_IS_NULL 26
#define WC_NOT_NULL 27

#define EXPR_VAL 28
#define EXPR_COL 29

class WhereC
{
    public:
    	int type;
    	int exprType;
 		string col;
        Value eval;
        string ecol;       
};

#endif