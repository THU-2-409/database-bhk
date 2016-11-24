#include <iostream>
#include "./utils/ByteArray.h"
#include "./table/Table.h"
#include <string>

using namespace std;

int main()
{
    string s("Hello world!");
    ByteArray a(s.c_str(), s.size());
    cout << a[11] << endl;
    cout << string("\0hell\0o world!") << endl;
    return 0;
}
