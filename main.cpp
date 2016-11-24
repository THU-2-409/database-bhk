#include <iostream>
#include "./utils/ByteArray.h"
#include "./table/Table.h"
#include <string>

using namespace std;

ByteArray fun()
{
    string s("Hello world!");
    ByteArray a(s.c_str(), s.size());
    return a;
}

int main()
{
    ByteArray b = fun();
    cout << b[11] << endl;
    return 0;
}
