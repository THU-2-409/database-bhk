%{
#include <stdio.h>
#include <string>
#include "../table/Table.h"
#include "func.h"
#include <vector>
#include "Value.h"
#include "WhereC.h"
#include "SetC.h"
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
            Table table;
            if (!table.open(path))
            {
                perror("Table open");
            }
            printf("%s\n", table.getSchema().c_str());
            if (table.close())
            {
                perror("Table close");
            }
        }
        |   P_INSERT P_INTO tbName P_VALUES valueLists
        |   P_DELETE P_FROM tbName P_WHERE whereClause
        |   P_UPDATE P_FROM tbName P_SET setClause P_WHERE whereClause
        |   P_SELECT selector P_FROM tableList P_WHERE whereClause
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
                    $$.defs.assign($1.defs.begin(), $1.defs.begin());
                    if($3.def.type == COL_KEY_T)
                    {
                        int size = $$.defs.size();
                        for(int i = 0; i < size; i++)
                        {
                            if($$.defs[i].name == $3.def.name)
                            {
                                $$.defs[i].type = COL_KEY_T;
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
                    $$.def.type = COL_REG_T;
                    $$.str = $1.str + " " + $2.str;
                }
            |   colName type P_NOT P_NULL
                {
                    $$.def.name = $1.str;
                    $$.def.size = $2.val;
                    $$.def.type = COL_NOT_NULL_T;
                    $$.str = $1.str + " " + $2.str + " NOT NULL";
                }
            |   P_PRIMARY P_KEY '(' colName ')'
                {
                    $$.def.name = $4.str;
                    $$.def.type = COL_KEY_T;
                    $$.str = string("PRIMARY KEY (") + $4.str + ")";
                }
            ;

type        :   P_INT '(' VALUE_INT ')'
                {
                    $$.val = $3.val * 4;
                    char buf[32];
                    sprintf(buf, "INT(%d)", $3.val);
                    $$.str = string(buf);
                }
            |   P_VARCHAR '(' VALUE_INT ')'
                {
                    $$.val = $3.val;
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
                    $$.vlists.assign($1.vlists.begin(), $1.vlists.end());
                    $$.vlists.push_back($4.vlist);
                }
            ;

valueList   :   value
                {
                    $$.vlist.push_back($1.v);
                }
            |   valueList ',' value
                {
                    $$.vlist.assign($1.vlist.begin(), $1.vlist.end());
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
                    wc.col = $1.str;
                    $$.wclist.push_back(wc);
                }
            |   col P_IS P_NOT P_NULL
                {
                    WhereC wc;
                    wc.type = WC_NOT_NULL;
                    wc.col = $1.str;
                    $$.wclist.push_back(wc);
                }
            ;

whereClause :   whereItem
            {
                $$.wclist = $1.wclist;
            }
            |   whereClause P_AND whereItem
            {
                $$.wclist.assign($1.wclist.begin(), $1.wclist.end());
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
                    $$.sclist.assign($1.sclist.begin(), $1.sclist.end());
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
                    $$.clist.assign($1.clist.begin(), $1.clist.end());
                }
            ;

colList     :   col
                {
                    $$.clist.push_back($1.str);
                }
            |   colList ',' col
                {
                    $$.clist.assign($1.clist.begin(), $1.clist.end());
                    $$.clist.push_back($3.str);
                }
            ;

tableList   :   tbName
                {
                    $$.clist.push_back($1.str);
                }
            |   tableList ',' tbName
                {
                    $$.clist.assign($1.clist.begin(), $1.clist.end());
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
int main()
{
    return yyparse();
}
void yyerror(const char* s)
{
    fprintf(stderr,"%s",s);
}
int yywrap()
{
    return 1;
}
