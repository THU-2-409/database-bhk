#ifndef _DB_FUNC_H_
#define _DB_FUNC_H_

#include <stdio.h>
#include <dirent.h>
#include "../table/Table.h"
#include <vector>
#include "Value.h"
#include "WhereC.h"
#include "SetC.h"

using namespace std;

void listDirectories()
{
    DIR *dp = opendir(".");
    if (NULL == dp)
    {
        perror("opendir");
        return ;
    }
    struct dirent * dirp;
    while ((dirp = readdir(dp)) != NULL)
        if (4 == dirp->d_type && strcmp(".", dirp->d_name)
                && strcmp("..", dirp->d_name))
        {
            printf("%s\n", dirp->d_name);
        }
    closedir(dp);
}

void listTables(const char * path)
{
    DIR *dp = opendir(path);
    if (NULL == dp)
    {
        perror("opendir");
        return ;
    }
    struct dirent * dirp;
    while ((dirp = readdir(dp)) != NULL)
        if (8 == dirp->d_type)
        {
            printf("%s\n", dirp->d_name);
        }
    closedir(dp);
}

void deleteTables(const char* path)
{
    DIR *dp = opendir(path);
    if (NULL == dp)
    {
        perror("opendir");
        return ;
    }
    struct dirent * dirp;
    while ((dirp = readdir(dp)) != NULL)
        if (8 == dirp->d_type)
        {
            string f = path;
            f += "/";
            f += dirp->d_name;
            if(remove(f.c_str()))
                perror("remove");
        }
    closedir(dp);
}

int table_open(Table &table, string path)
{
    if (!table.open(path))
    {
        dperr("Table open");
        return -1;
    }
    return 0;
}
int table_close(Table &table)
{
    int r;
    if ((r = table.close()))
    {
        dperr("Table close(%d)", r);
        return -1;
    }
    return 0;
}

int intcmp(int a, int b)
{
    if (a == b) return 0;
    else return a < b ? -1 : 1;
}
// 固定值限制
bool checkCond(RecordData &data, vector<WhereC> &cond)
{
    for (int i = 0, cmp; i < cond.size(); ++i)
    {
        bool flag = false;
        string &col = cond[i].col;
        pair<bool, int> t0;
        pair<bool, string> t1;
        if (WC_IS_NULL != cond[i].type && WC_NOT_NULL != cond[i].type)
        {
            switch (cond[i].eval.type)
            {
                case VAL_INT:
                    t0 = data.getInt(col);
                    if (!t0.first) return false;
                    cmp = intcmp(t0.second, cond[i].eval.val);
                    break;
                case VAL_STRING:
                    t1 = data.getString(col);
                    if (!t1.first) return false;
                    cmp = t1.second.compare(cond[i].eval.str);
                    break;
                case VAL_NULL:
                    break;
            }
        }
        switch (cond[i].type)
        {
            case WC_IS_NULL:
                flag = data.isNULL(col);
                break;
            case WC_NOT_NULL:
                flag = !data.isNULL(col);
                break;
            case WC_NOT_EQU:
                flag = cmp != 0;
                break;
            case WC_LEQ:
                flag = cmp <= 0;
                break;
            case WC_GEQ:
                flag = cmp >= 0;
                break;
            case WC_LE:
                flag = cmp < 0;
                break;
            case WC_GR:
                flag = cmp > 0;
                break;
        }
        if (!flag) return false;
    }
    return true;
}

/*
 * 滤去等于固定值的条件，产生用于find的集合
 */

RecordData whereCeqsFilter(vector<WhereC> & wclist)
{
    RecordData eqd;
    vector<WhereC> vcond;
    for (int i = 0; i < wclist.size(); ++i)
    {
        WhereC &c = wclist[i];
        if (EXPR_VAL == c.exprType && WC_EQU == c.type)
        {
            switch (c.eval.type)
            {
                case VAL_INT:
                    eqd.setInt(c.col, c.eval.val);
                    break;
                case VAL_STRING:
                    eqd.setString(c.col, c.eval.str);
                    break;
                case VAL_NULL:
                    eqd.setNULL(c.col);
                    break;
            }
        } else 
            if (EXPR_VAL == c.exprType) vcond.push_back(c);
    }
    wclist.swap(vcond);
    return eqd;
}

void updateData(RecordData & data, vector<SetC> sclist)
{
    for (int i = 0; i < sclist.size(); ++i)
    {
        switch (sclist[i].val.type)
        {
            case VAL_STRING:
                data.setString(sclist[i].col, sclist[i].val.str);
                break;
            case VAL_INT:
                data.setInt(sclist[i].col, sclist[i].val.val);
                break;
            case VAL_NULL:
                data.setNULL(sclist[i].col);
                break;
        }
    }
}

bool checkRecCons(RecordData & data, Table & table)
{
    // TODO
    return true;
}

RecordData genPKupdCheck(SetC s)
{
    RecordData rd;
    switch (s.val.type)
    {
        case VAL_INT:
            rd.setInt(s.col, s.val.val);
            break;
        case VAL_STRING:
            rd.setString(s.col, s.val.str);
            break;
        default:
            errno = 22;
            return rd;
    }
    return rd;
}

#endif
