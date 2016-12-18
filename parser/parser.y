%{
#include <stdio.h>
#include <string>
#include "../table/Table.h"
#include "func.h"
#include "lex.yy.c"
#include <vector>
struct yyStruType
{
    int type;

    int val;

    string str;

    std::vector<ColDef> defs;
};
#define YYSTYPE yyStruType  
int yyparse(void);
%}
%token P_DATABASE P_DATABASES   P_TABLE   P_TABLES  P_SHOW    P_CREATE
%token P_DROP P_USE P_PRIMARY P_KEY P_NOT P_NULL
%token P_INSERT   P_INTO    P_VALUES  P_DELETE  P_FROM    P_WHERE
%token P_UPDATE   P_SET P_SELECT  P_IS  P_INT P_VARCHAR
%token P_DESC P_INDEX   P_AND
%token IDENTIFIER VALUE_INT VALUE_STRING
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

tbStmt  :   P_CREATE P_TABLE tbName '(' fieldList ')'
        {
            string schema = string("CREATE TABLE ") + $3.str;
            schema += " (" + $5.str + ");";
            TableHeader header(schema, $5.defs);
            if (!Table::createFile(header, path))
            {
                printf("error: %d %s\n", errno, strerror(errno));
            }
        }
        |   P_DROP P_TABLE tbName
        |   P_DESC tbName
        |   P_INSERT P_INTO tbName P_VALUES valueLists
        |   P_DELETE P_FROM tbName P_WHERE whereClause
        |   P_SELECT selector P_FROM tableList P_WHERE whereClause
        ;

idxStmt :   P_CREATE P_INDEX tbName '(' colName ')'
        |   P_DROP P_INDEX tbName '(' colName ')'
        ;




%%
int main()
{
    return yyparse();
}
void yyerror(char* s)
{
    fprintf(stderr,"%s",s);
}
int yywrap()
{
    return 1;
}
