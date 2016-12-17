%{
#include <stdio.h>
#include "lex.yy.c"
union yyUnionType
{
    int val;
    char * str;
};
#define YYSTYPE yyUnionType  
int yyparse(void);
%}
%token P_DATABASE P_DATABASES   P_TABLE   P_TABLES  P_SHOW    P_CREATE
%token P_DROP P_USE P_PRIMARY P_KEY P_NOT P_NULL
%token P_INSERT   P_INTO    P_VALUES  P_DELETE  P_FROM    P_WHERE
%token P_UPDATE   P_SET P_SELECT  P_IS  P_INT P_VARCHAR
%token P_DESC P_INDEX   P_AND
%token IDENTIFIER VALUE_INT VALUE_STRING
%%
command : exp {printf("%d\n",$1);}

exp: exp PLUS term {$$ = $1 + $3;}
    |exp MINUS term {$$ = $1 - $3;}
    |term {$$ = $1;}
    ;
term : term TIMES factor {$$ = $1 * $3;}
    |term DIVIDE factor {$$ = $1/$3;}
    |factor {$$ = $1;}
    ;
factor : INTEGER {$$ = $1;}
    | LP exp RP {$$ = $2;}
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
