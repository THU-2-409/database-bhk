#ifndef _PRINT_WG_H_
#define _PRINT_WG_H_

#include <stdio.h>
#include <string>
using namespace std;

class PrintWg
{
public:
    static int cnt;

    static void pw(int x)
    {
        if (cnt ++) printf("|");
        printf("%d", x);
    }
    static void pw(const char * s)
    {
        if (cnt ++) printf("|");
        printf("%s", s);
    }
    static void pw(string s)
    {
        if (cnt ++) printf("|");
        printf("'%s'", s.c_str());
    }
    static void pwln()
    {
        cnt = 0;
        printf("\n");
    }
private:
    PrintWg() {}
};

int PrintWg:: cnt = 0;

#endif
