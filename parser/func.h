#ifndef _DB_FUNC_H_
#define _DB_FUNC_H_

#include <stdio.h>
#include <dirent.h>
#include "../utils.h"
#include "ColStr.h"
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
bool checkCond(RecordData &data,
        vector<WhereC> &cond,
        bool sel2 = false)
{
    for (int i = 0, cmp; i < cond.size(); ++i)
    {
        bool flag = false;
        string &col = sel2 ? cond[i].lcol.second : cond[i].col;
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

vector<ColStr> genSelHeader(vector<ColStr> raw,
        vector<TableHeader> hs,
        vector<string> tbnames,
        vector<int> & vtype)
{
    if (raw[0].second == "*")
    {
        raw.clear();
        for (int i = 0; i < hs.size(); ++i)
            for (int j = 0; j < hs[i].getColNums(); ++j)
            {
                string name = hs[i].getName(j);
                raw.push_back(make_pair(tbnames[i], name));
                if (i + j > 0) printf("|");
                printf("%s.%s", tbnames[i].c_str(), name.c_str());
                vtype.push_back(hs[i].getType(j));
            }
        printf("\n");
    }
    else
    {
        for (int i = 0; i < raw.size(); ++i)
        {
            if (raw[i].first == string("#"))
            {
                for (int j = 0; j < hs.size(); ++j)
                    if (hs[j].hasName(raw[i].second))
                    {
                        raw[i].first = tbnames[j];
                        break;
                    }
            }
            printf("%s.%s%c", raw[i].first.c_str(),
                    raw[i].second.c_str(),
                    (i < raw.size() - 1) ? '|' : '\n');
            for (int j = 0; j < hs.size(); ++j)
                if (tbnames[j] == raw[i].first)
                {
                    vtype.push_back(hs[j].getType(raw[i].second));
                    break;
                }
        }
    }
    return raw;
}

void printRecData(RecordData & data,
        vector<ColStr> & hs,
        vector<int> & vtype,
        const string & name)
{
    for (int i = 0; i < hs.size(); ++i)
        if (hs[i].first == name &&
                data.hasName(hs[i].second))
        {
            string & col = hs[i].second;
            if (data.isNULL(col))
            {
                PrintWg::pw("NULL");
                continue;
            }
            switch (vtype[i])
            {
                case COL_TYPE_VINT:
                {
                    pair<bool, int> t = data.getInt(col);
                    PrintWg::pw(t.second);
                    break;
                }
                case COL_TYPE_VSTR:
                {
                    pair<bool, string> t = data.getString(col);
                    PrintWg::pw(t.second);
                    break;
                }
                default:
                    printf("ERROR!\n");
                    break;
            }
        }
}

void _fixSharp(ColStr & wc, TableHeader & h, string & tb)
{
    if (wc.first == "#" && h.hasName(wc.second))
        wc.first = tb;
}

void fixSharpWc(vector<WhereC> & wclist,
        vector<TableHeader> & hs,
        vector<string> & tbnames)
{
    for (int i = 0; i < hs.size(); ++i)
    {
        for (int j = 0; j < wclist.size(); ++j)
        {
            _fixSharp(wclist[j].lcol, hs[i], tbnames[i]);
            _fixSharp(wclist[j].rcol, hs[i], tbnames[i]);
        }
    }
}

RecordData eqfilter(vector<WhereC> & wclist, string tb)
{
    RecordData res;
    for (int i = 0; i < wclist.size(); ++i)
    {
        if (EXPR_VAL == wclist[i].exprType &&
                WC_EQU == wclist[i].type &&
                tb == wclist[i].lcol.first)
        {
            switch (wclist[i].eval.type)
            {
                case VAL_INT:
                    res.setInt(wclist[i].lcol.second,
                            wclist[i].eval.val);
                    break;
                case VAL_STRING:
                    res.setString(wclist[i].lcol.second,
                            wclist[i].eval.str);
                    break;
                case VAL_NULL:
                    res.setNULL(wclist[i].lcol.second);
                    break;
            }
        }
    }
    return res;
}

vector<WhereC> nefilter(vector<WhereC> wclist, string tb)
{
    vector<WhereC> res;
    for (int i = 0; i < wclist.size(); ++i)
    {
        if (EXPR_VAL == wclist[i].exprType &&
                tb == wclist[i].lcol.first &&
                WC_EQU != wclist[i].type)
        {
            res.push_back(wclist[i]);
        }
    }
    return res;
}

vector<RecordData> find2(Table & table,
        vector<WhereC> wclist,
        string tb)
{
    RecordData eqd = eqfilter(wclist, tb);
    vector<Record> meta = table.find(eqd);
    vector<WhereC> cond = nefilter(wclist, tb);
    vector<RecordData> res;
    for (int i = 0; i < meta.size(); ++i)
    {
        RecordData data = meta[i].getData();
        if (checkCond(data, cond, true))
            res.push_back(data);
    }
    return res;
}

vector<WhereC> uofilter(vector<WhereC> wclist)
{
    vector<WhereC> res;
    for (int i = 0; i < wclist.size(); ++i)
        if (EXPR_COL == wclist[i].exprType)
            res.push_back(wclist[i]);
    return res;
}

vector<int> gCondType(vector<WhereC> wclist,
        vector<TableHeader> & hs,
        vector<string> & tbnames)
{
    vector<int> res;
    for (int i = 0; i < wclist.size(); ++i)
        for (int j = 0; j < hs.size(); ++j)
            if (tbnames[j] == wclist[i].lcol.first)
            {
                res.push_back(hs[j].getType(wclist[i].lcol.second));
                break;
            }
    return res;
}

bool checkUnion(RecordData & d0, RecordData & d1,
        vector<WhereC> & cond,
        vector<int> & type,
        vector<string> & tbs)
{
    pair<bool, ByteArray> t0, t1;
    RecordData *lp, *rp;
    for (int i = 0, cmp; i < cond.size(); ++i)
    {
        if (cond[i].lcol.first == tbs[0])
        {
            lp = &d0;
            rp = &d1;
        }
        else
        {
            lp = &d1;
            rp = &d0;
        }
        t0 = lp->getBA(cond[i].lcol.second);
        t1 = rp->getBA(cond[i].rcol.second);
        if (!t0.first || !t1.first) return false;
        switch (type[i])
        {
            case COL_TYPE_VINT:
                cmp = t0.second.intCmp(t1.second);
                break;
            case COL_TYPE_VSTR:
                cmp = t0.second.strCmp(t1.second);
                break;
            default:
                printf("Error col type!\n");
                return false;
        }
        bool flag;
        switch (cond[i].type)
        {
            case WC_EQU:
                flag = cmp == 0;
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
            case WC_IS_NULL:
            case WC_NOT_NULL:
            default:
                printf("Error wc type!\n");
                return false;
        }
        if (!flag) return false;
    }
    return true;
}

void printUni(RecordData & d0, RecordData & d1,
        vector<ColStr> & pcs,
        vector<int> & ptype,
        vector<string> & tbs)
{
    RecordData * p;
    for (int i = 0; i < pcs.size(); ++i)
    {
        p = tbs[0] == pcs[i].first ? &d0 : &d1;
        if (p->isNULL(pcs[i].second))
        {
            PrintWg::pw("NULL");
            continue;
        }
        switch (ptype[i])
        {
            case COL_TYPE_VINT:
            {
                pair<bool, int> t = p->getInt(pcs[i].second);
                PrintWg::pw(t.second);
                break;
            }
            case COL_TYPE_VSTR:
            {
                pair<bool, string> t = p->getString(pcs[i].second);
                PrintWg::pw(t.second);
                break;
            }
        }
    }
}

#endif
