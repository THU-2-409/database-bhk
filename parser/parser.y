%{
#include <stdio.h>
#include <string>
#include "../table/Table.h"
#include <vector>
#include "ColStr.h"
#include "Value.h"
#include "WhereC.h"
#include "SetC.h"
#include "func.h"
#include <set>
struct yyStruType
{
    int type;

    int val;

    string str;

    ColDef def;

    ColStr col;

    std::vector<ColDef> defs;

    Value v;

    std::vector<Value> vlist;

    std::vector< std::vector<Value> > vlists;

    std::vector<WhereC> wclist;

    std::vector<SetC> sclist;

    std::vector<string> clist;

    std::vector<ColStr> cslist;
};
#define YYSTYPE yyStruType  

string dbPath(".");
void yyerror(const char* s);
#include "lex.yy.c"
int yyparse(void);

%}
%token P_DATABASE P_DATABASES   P_TABLE   P_TABLES  P_SHOW    P_CREATE
%token P_DROP P_USE P_PRIMARY P_KEY P_NOT P_NULL
%token P_INSERT   P_INTO    P_VALUES  P_DELETE  P_FROM    P_WHERE
%token P_UPDATE   P_SET P_SELECT  P_IS  P_INT P_VARCHAR
%token P_DESC P_INDEX   P_AND
%token IDENTIFIER VALUE_INT VALUE_STRING
%token OP_NEQ OP_GEQ OP_LEQ
%%
program :   program stmt
        |   /* empty */
        ;

stmt    :   sysStmt ';'
        |   dbStmt ';'
        |   tbStmt ';'
        |   idxStmt ';'
        ;

sysStmt :   P_SHOW P_DATABASES
        {
            listDirectories();
        }
        ;

dbStmt  :   P_CREATE P_DATABASE dbName
        {
            if (mkdir($3.str.c_str(), 0775))
            {
                perror("mkdir err");
            }
        }
        |   P_DROP P_DATABASE dbName
        {
            if ($3.str == dbPath) dbPath = string(".");
            deleteTables($3.str.c_str());
            if (rmdir($3.str.c_str()))
            {
                perror("rmdir err");
            }
        }
        |   P_USE dbName
        {
            dbPath = $2.str;
        }
        |   P_SHOW P_TABLES
        {
            listTables(dbPath.c_str());
        }
        ;

tbStmt  :   P_CREATE P_TABLE tbName '(' fieldList ')'
        {
            string schema = string("CREATE TABLE ") + $3.str;
            schema += " (" + $5.str + ");";
            TableHeader header(schema, $5.defs);
            string path = dbPath + "/" + $3.str;
            if (!Table::createFile(header, path))
            {
                perror("Table:: createFile");
            }
        }
        |   P_DROP P_TABLE tbName
        {
            string path = dbPath + "/" + $3.str;
            if (Table::deleteFile(path))
            {
                perror("Table:: deleteFile");
            }
        }
        |   P_DESC tbName /* 查看表的schema */
        {
            string path = dbPath + "/" + $2.str;
            Table table1;
            table_open(table1, path);
            printf("%s\n", table1.getSchema().c_str());
            table_close(table1);
        }
        |   P_INSERT P_INTO tbName P_VALUES valueLists
        {
            string path = dbPath + "/" + $3.str;
            Table table;
            table_open(table, path);
            TableHeader & header = table.getHeader();

            TableInfo & info = table.getInfo();
            int keyID = info.getKey();
            //printf("keyID: %d\n", keyID);
            set<int> ikey;
            set<string> skey;
            if(keyID != -1)
            { 
                int keytype = header.getType(keyID);
                string keyname = header.getName(keyID);
                RecordData rdtemp;
                vector<Record> temp = table.find(rdtemp);
                int size = temp.size();
                if(keytype == COL_TYPE_VINT)
                {
                    for(int i = 0; i < size; i++)
                    {
                        RecordData rd = temp[i].getData();
                        ikey.insert(rd.getInt(keyname).second);
                    }
                }
                else
                {
                    for(int i = 0; i < size; i++)
                    {
                        RecordData rd = temp[i].getData();
                        skey.insert(rd.getString(keyname).second);
                    }
                }             
            }

            for (int k = 0; k < $5.vlists.size(); ++k)
            {
                //printf("1go\n");
                RecordData tmp;
                vector<Value> &rda = $5.vlists[k];

                bool flag = true;
                for (int i = 0; i < rda.size(); ++i)
                {
                    //printf("2go%d\n", i);
                    Value &v = rda[i];
                    switch(v.type)
                    {
                        case VAL_INT:
                            if(header.getType(i) != COL_TYPE_VINT)
                                flag = false;
                            break;
                        case VAL_STRING:
                            if(header.getType(i) != COL_TYPE_VSTR)
                                flag = false;
                            break;
                    }
                    if(!flag)
                        break;
                }
                if(!flag)
                {
                    printf("record type error, please check your insert\n");
                    continue;
                }

                if(keyID != -1)
                {
                    int keytype = header.getType(keyID);
                    if(keytype == COL_TYPE_VINT)
                    {
                        if(ikey.find(rda[keyID].val) != ikey.end())
                        {
                            printf("record's primary key already exist\n");
                            continue;                      
                        }
                        else
                            ikey.insert(rda[keyID].val);   
                    }
                    else
                    {
                        if(skey.find(rda[keyID].str) != skey.end())
                        {
                            printf("record's primary key already exist\n");
                            continue;                      
                        }
                        else
                            skey.insert(rda[keyID].str);  
                    }
                }

                bool nullflag = true;
                for (int i = 0; i < rda.size(); ++i)
                {
                    //printf("2go%d\n", i);
                    Value &v = rda[i];
                    string name = header.getName(i);
                    switch (v.type)
                    {
                        case VAL_INT:
                            //printf("pre set %d\n", v.val);
                            tmp.setInt(name, v.val);
                            //printf("set %d\n", tmp.getInt(name).second);
                            break;
                        case VAL_STRING:
                            //printf("pre set %s\n", v.str.c_str());
                            tmp.setString(name, v.str);
                            //printf("set %s\n", tmp.getString(name).second.c_str());
                            break;
                        case VAL_NULL:
                            if(header.getConstraint(i) != COL_REG_T)
                            {
                                nullflag = false;
                                printf("record field can't set null\n");
                            }
                            else
                            {
                                tmp.setNULL(name);
                            }
                            break;
                    }
                    if(!nullflag)
                        break;
                }
                //printf("3go\n");
                if(!nullflag)
                    continue;
                
                table.insert(tmp);
            }
            table_close(table);
        }
        |   P_DELETE P_FROM tbName P_WHERE whereClause
        {
            string path = dbPath + "/" + $3.str;
            Table table;
            table_open(table, path);
            RecordData eqd = whereCeqsFilter($5.wclist);
            vector<Record> rs = table.find(eqd);
            for (int i = 0; i < rs.size(); ++i)
            {
                RecordData data = rs[i].getData();
                if (checkCond(data, $5.wclist))
                {
                    DataPage pg(rs[i].getPageID(), &table);
                    pg.removeRecord(rs[i].getOffset());
                }
            }
            table_close(table);
        }
        |   P_UPDATE tbName P_SET setClause P_WHERE whereClause
        {
            string path = dbPath + "/" + $2.str;
            Table table;
            table_open(table, path);

            TableHeader & header = table.getHeader();
            TableInfo & info = table.getInfo();
            int keyID = info.getKey();
            // 非空检查
            bool nullerr = false, errhalt = false, set_pk = false;
            for (int i = 0; i < $4.sclist.size(); ++i)
            {
                int ty = header.getConstraint($4.sclist[i].col);
                if (COL_NOT_NULL_T == ty || COL_KEY_T == ty)
                    if (VAL_NULL == $4.sclist[i].val.type)
                    {
                        errhalt = nullerr = true;
                        break;
                    }
                if (COL_KEY_T == ty) 
                {
                    set_pk = true;
                    RecordData pks = genPKupdCheck($4.sclist[i]);
                    vector<Record> vec = table.find(pks);
                    if (vec.size())
                    {
                        errhalt = true;
                        printf("error: Primary Key Constraint\n");
                        break;
                    }
                }
            }
            if (nullerr) printf("error: NULL Constraint\n");
            if (errhalt) 
            {
                table_close(table);
                break;
            }
            RecordData eqd = whereCeqsFilter($6.wclist);
            vector<Record> rs = table.find(eqd);
            vector<Record> res;
            for (int i = 0; i < rs.size(); ++i)
            {
                RecordData data = rs[i].getData();
                if (checkCond(data, $6.wclist))
                {
                    res.push_back(rs[i]);
                }
            }
            if (res.size() > 1 && set_pk)
            {
                printf("error: Primary Key Constraint\n");
                table_close(table);
                break;
            }
            for (int i = 0; i < res.size(); ++i)
            {
                RecordData data = res[i].getData();
                updateData(data, $4.sclist);
                res[i].setData(data);
            }
            table_close(table);
        }
        |   P_SELECT selector P_FROM tableList P_WHERE whereClause
        {
            // 计算header
            vector<ColStr> p_heads;
            vector<int> p_vtypes;
            {
                vector<TableHeader> hs;
                Table table;
                for (int i = 0; i < $4.clist.size(); ++i)
                {
                    string path = dbPath + "/" + $4.clist[i];
                    table_open(table, path);
                    hs.push_back(table.getHeader());
                    table_close(table);
                }
                p_heads = genSelHeader($2.cslist, hs,
                    $4.clist, p_vtypes);
            }
            // 分类套路
            switch ($4.clist.size())
            {
                case 1:
                {
                    Table table;
                    string name = $4.clist[0];
                    string path = dbPath + "/" + name;
                    table_open(table, path);
                    vector<WhereC> vcond = $6.wclist;
                    RecordData eqd = whereCeqsFilter(vcond);
                    vector<Record> meta = table.find(eqd);
                    for (int i = 0; i < meta.size(); ++i)
                    {
                        RecordData data = meta[i].getData();
                        if (checkCond(data, vcond))
                        {
                            printRecData(data, p_heads, p_vtypes, name);
                            PrintWg::pwln();
                        }
                    }
                    table_close(table);
                }
                    break;
                case 2:
                {
                    printf("error: not implemented\n");
                }
                    break;
                default:
                    printf("error: 不支持\n");
                    break;
            }
        }
        ;

idxStmt :   P_CREATE P_INDEX tbName '(' colName ')'
        |   P_DROP P_INDEX tbName '(' colName ')'
        ;

fieldList   :   field
                {
                    $$.defs.push_back($1.def);
                    $$.str = $1.str;
                }
            |   fieldList ',' field
                {
                    $$.str = $1.str + ", " + $3.str;
                    //$$.defs.assign($1.defs.begin(), $1.defs.begin());
                    $$.defs = $1.defs;
                    if($3.def.constraint == COL_KEY_T)
                    {
                        int size = $$.defs.size();
                        for(int i = 0; i < size; i++)
                        {
                            if($$.defs[i].name == $3.def.name)
                            {
                                $$.defs[i].constraint = COL_KEY_T;
                                break;
                            }
                        }
                    }
                    else
                        $$.defs.push_back($3.def);
                }
            ;

field       :   colName type
                {
                    $$.def.name = $1.str;
                    $$.def.size = $2.val;
                    $$.def.type = $2.type;
                    $$.def.constraint = COL_REG_T;
                    $$.str = $1.str + " " + $2.str;
                }
            |   colName type P_NOT P_NULL
                {
                    $$.def.name = $1.str;
                    $$.def.size = $2.val;
                    $$.def.type = $2.type;
                    $$.def.constraint = COL_NOT_NULL_T;
                    $$.str = $1.str + " " + $2.str + " NOT NULL";
                }
            |   P_PRIMARY P_KEY '(' colName ')'
                {
                    $$.def.name = $4.str;
                    $$.def.constraint = COL_KEY_T;
                    $$.str = string("PRIMARY KEY (") + $4.str + ")";
                }
            ;

type        :   P_INT '(' VALUE_INT ')'
                {
                    $$.val = 4;
                    $$.type = COL_TYPE_VINT;
                    char buf[32];
                    sprintf(buf, "INT(%d)", $3.val);
                    $$.str = string(buf);
                }
            |   P_VARCHAR '(' VALUE_INT ')'
                {
                    $$.val = $3.val;
                    $$.type = COL_TYPE_VSTR;
                    char buf[32];
                    sprintf(buf, "CHAR(%d)", $3.val);
                    $$.str = string(buf);
                }
            ;

valueLists  :   '(' valueList ')'
                {
                    $$.vlists.push_back($2.vlist);
                }
            |   valueLists ',' '(' valueList ')'
                {
                    //$$.vlists.assign($1.vlists.begin(), $1.vlists.end());
                    $$.vlists = $1.vlists;
                    $$.vlists.push_back($4.vlist);
                }
            ;

valueList   :   value
                {
                    $$.vlist.push_back($1.v);
                }
            |   valueList ',' value
                {
                    //$$.vlist.assign($1.vlist.begin(), $1.vlist.end());
                    $$.vlist = $1.vlist;
                    $$.vlist.push_back($3.v);
                }
            ;

value       :   VALUE_INT
                {
                    $$.v.type = VAL_INT;
                    $$.v.val = $1.val;
                }
            |   VALUE_STRING
                {
                    $$.v.type = VAL_STRING;
                    $$.v.str = $1.str;
                }
            |   P_NULL
                {
                    $$.v.type = VAL_NULL;
                }
            ;

whereItem   :   col op expr
                {
                    WhereC wc;
                    wc.type = $2.type;
                    wc.exprType = $3.type;
                    wc.col = $1.str;
                    wc.lcol = $1.col;
                    if($3.type == EXPR_VAL)
                        wc.eval = $3.v;
                    else
                    {
                        wc.ecol = $3.str;
                        wc.rcol = $3.col;
                    }
                    $$.wclist.push_back(wc);
                }
            |   col P_IS P_NULL
                {
                    WhereC wc;
                    wc.type = WC_IS_NULL;
                    wc.exprType = EXPR_VAL;
                    wc.col = $1.str;
                    wc.lcol = $1.col;
                    $$.wclist.push_back(wc);
                }
            |   col P_IS P_NOT P_NULL
                {
                    WhereC wc;
                    wc.type = WC_NOT_NULL;
                    wc.exprType = EXPR_VAL;
                    wc.col = $1.str;
                    wc.lcol = $1.col;
                    $$.wclist.push_back(wc);
                }
            ;

whereClause :   whereItem
            {
                $$.wclist = $1.wclist;
            }
            |   whereClause P_AND whereItem
            {
                //$$.wclist.assign($1.wclist.begin(), $1.wclist.end());
                $$.wclist = $1.wclist;
                $$.wclist.push_back($3.wclist.at(0));
            }

col         :   colName
                {
                    $$.str = $1.str;
                    $$.col.first = "#";
                    $$.col.second = $1.str;
                }
            |   tbName '.' colName
                {
                    $$.str = $1.str + "." + $3.str;
                    $$.col.first = $1.str;
                    $$.col.second = $3.str;
                }   
            ;

op          :   '='     { $$.type = WC_EQU; }
            |   OP_NEQ  { $$.type = WC_NOT_EQU; }
            |   OP_LEQ  { $$.type = WC_LEQ; }
            |   OP_GEQ  { $$.type = WC_GEQ; }
            |   '<'     { $$.type = WC_LE; }
            |   '>'     { $$.type = WC_GR; }
            ;

expr        :   value
                {
                    $$.type = EXPR_VAL;
                    $$.v = $1.v;
                }
            |   col
                {
                    $$.type = EXPR_COL;
                    $$.str = $1.str;
                    $$.col = $1.col;
                }
            ;   

setClause   :   colName '=' value
                {
                    SetC sc;
                    sc.col = $1.str;
                    sc.val = $3.v;
                    $$.sclist.push_back(sc);
                }
            |   setClause ',' colName '=' value
                {
                    //$$.sclist.assign($1.sclist.begin(), $1.sclist.end());
                    $$.sclist = $1.sclist;
                    SetC sc;
                    sc.col = $3.str;
                    sc.val = $5.v;
                    $$.sclist.push_back(sc);
                }
            ;

selector    :   '*'
                {
                    $$.clist.push_back("*");
                    $$.cslist.push_back(genColStr("#", "*"));
                }
            |   colList
                {
                    //$$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist = $1.clist;
                    $$.cslist = $1.cslist;
                }
            ;

colList     :   col
                {
                    $$.clist.push_back($1.str);
                    $$.cslist.push_back($1.col);
                }
            |   colList ',' col
                {
                    //$$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist = $1.clist;
                    $$.clist.push_back($3.str);
                    $$.cslist = $1.cslist;
                    $$.cslist.push_back($3.col);
                }
            ;

tableList   :   tbName
                {
                    $$.clist.push_back($1.str);
                }
            |   tableList ',' tbName
                {
                    //$$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist = $1.clist;
                    $$.clist.push_back($3.str);
                }
            ;

dbName      :   IDENTIFIER
                {
                    $$.str = $1.str;
                }
            ;

tbName      :   IDENTIFIER
                {
                    $$.str = $1.str;
                }
            ;

colName     :   IDENTIFIER
                {
                    $$.str = $1.str;
                }
            ;   


%%
int main(int args, char *argv[])
{
    if (args > 1)
        freopen(argv[1], "r", stdin);
    int ret = yyparse();
    if (args > 1) fclose(stdin);
    return ret;
}
void yyerror(const char* s)
{
    fprintf(stderr,"%s",s);
}
int yywrap()
{
    return 1;
}
// vim: ai
