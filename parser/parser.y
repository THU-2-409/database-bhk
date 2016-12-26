%{
#include <stdio.h>
#include <string>
#include "../table/Table.h"
#include <vector>
#include "Value.h"
#include "WhereC.h"
#include "SetC.h"
#include "func.h"
struct yyStruType
{
    int type;

    int val;

    string str;

    ColDef def;

    std::vector<ColDef> defs;

    Value v;

    std::vector<Value> vlist;

    std::vector< std::vector<Value> > vlists;

    std::vector<WhereC> wclist;

    std::vector<SetC> sclist;

    std::vector<string> clist;
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

                TableInfo & info = table.getInfo();
                int keyID = info.getKey();
                printf("keyID: %d\n", keyID);
                if(keyID != -1)
                {
                    switch (rda[keyID].type)
                    {
                        case VAL_INT:
                            tmp.setInt(header.getName(keyID), rda[keyID].val);
                            break;
                        case VAL_STRING:
                            tmp.setString(header.getName(keyID), rda[keyID].str);
                            break;
                    }
                    vector<Record> temp = table.find(tmp);
                    if(temp.size() > 0)
                    {
                        printf("record's primary key already exist\n");
                        continue;
                    }
                }

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
                            tmp.setNULL(name);
                            break;
                    }
                }
                //printf("3go\n");
                
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
        {   // 需要约束检查 TODO
            string path = dbPath + "/" + $2.str;
            Table table;
            table_open(table, path);
            RecordData eqd = whereCeqsFilter($6.wclist);
            vector<Record> rs = table.find(eqd);
            for (int i = 0; i < rs.size(); ++i)
            {
                RecordData data = rs[i].getData();
                if (checkCond(data, $6.wclist))
                {
                    updateData(data, $4.sclist);
                    rs[i].setData(data);
                }
            }
            table_close(table);
        }
        |   P_SELECT selector P_FROM tableList P_WHERE whereClause
        {
            //printf("fuck1\n");
            RecordData eqd;
            vector<WhereC> & vcond = $6.wclist;
            eqd = whereCeqsFilter(vcond);
            vector<pair<string, RecordData> > meta, res;
            vector<pair<string, string> > colsh;
            vector<int> coltype;
            bool allcol = "*" == $2.clist[0];
            //printf("allcol %d\n", allcol);
            if (!allcol)
            {
                coltype = vector<int>($2.clist.size(), 0);
                for (int i = 0; i < $2.clist.size(); ++i)
                    colsh.push_back(make_pair(string(), $2.clist[i]));
            }
            Table table;
            //printf("fuck3\n");
            for (int Ti = 0; Ti < $4.clist.size(); ++Ti)
            {
                string tb = $4.clist[Ti] + "\0";
                table_open(table, dbPath + "/" + tb);
                TableHeader &header = table.getHeader();
                if (allcol)
                {
                    coltype.insert(coltype.end(), header.getColNums(), 0);
                    for (int i = 0; i < header.getColNums(); ++i)
                        colsh.push_back(make_pair(tb, header.getName(i)));
                    /*for (int i = 0; i < header.getColNums(); ++i)
                          printf("colsh %s\n", colsh[i].second.c_str());*/
                }
                for (int i = 0; i < colsh.size(); ++i)
                    if (colsh[i].first == tb || (colsh[i].first.empty()
                        && 0 == Ti))
                    {
                        coltype[i] = header.getType(colsh[i].second);
                    }
                vector<Record> temp = table.find(eqd);
                printf("select temp %d\n",(int)temp.size());
                for (int i = 0; i < temp.size(); ++i)
                {
                    RecordData data = temp[i].getData();
                    if (checkCond(data, vcond))
                        meta.push_back(make_pair(tb, data));
                }
                table_close(table);
            }
            // 联合查询
            res = meta;
            // 输出 (! 单表查询)
            //printf("fuck4\n");
            for (int i = 0; i < colsh.size(); ++i)
            {            
                printf("%s", colsh[i].first.c_str());
                printf(".%s", colsh[i].second.c_str());
                printf("%c", (i < colsh.size() - 1) ? '|' : '\n');
            }
            //printf("fuck5\n");
            for (int i = 0; i < res.size(); ++i)
            {
                RecordData &data = res[i].second;
                for (int j = 0; j < colsh.size(); ++j)
                {
                    string name = colsh[j].second;
                    if (data.isNULL(name))
                    {
                        printf("null");
                    }
                    else
                    {
                        switch (coltype[j])
                        {
                            case COL_TYPE_VINT:
                                printf("%d", data.getInt(name).second);
                                break;
                            case COL_TYPE_VSTR:
                                printf("%s", data.getString(name)
                                    .second.c_str());
                                break;
                            default:
                                printf("?? type err ??");
                                break;
                        }
                    }
                    printf("%c", (j < colsh.size() - 1) ? '|' : '\n');
                }
            }
            //printf("fuck6\n");
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
                    sprintf(buf, "VARCHAR(%d)", $3.val);
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
                    if($3.type == EXPR_VAL)
                        wc.eval = $3.v;
                    else
                        wc.ecol = $3.str;
                    $$.wclist.push_back(wc);
                }
            |   col P_IS P_NULL
                {
                    WhereC wc;
                    wc.type = WC_IS_NULL;
                    wc.exprType = EXPR_VAL;
                    wc.col = $1.str;
                    wc.eval.type = VAL_NULL;
                    $$.wclist.push_back(wc);
                }
            |   col P_IS P_NOT P_NULL
                {
                    WhereC wc;
                    wc.type = WC_NOT_NULL;
                    wc.exprType = EXPR_VAL;
                    wc.col = $1.str;
                    wc.eval.type = VAL_NULL;
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
                }
            |   tbName '.' colName
                {
                    $$.str = $1.str + "." + $3.str;
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
                }
            |   colList
                {
                    //$$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist = $1.clist;
                }
            ;

colList     :   col
                {
                    $$.clist.push_back($1.str);
                }
            |   colList ',' col
                {
                    //$$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist = $1.clist;
                    $$.clist.push_back($3.str);
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
