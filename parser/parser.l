%{
#include <string.h>  
#include <string>
#include "parser.tab.h"  
extern yyStruType yylval;  
%}  
identifier [A-Za-z][_0-9A-Za-z]* 
int [0-9]+  
string '[^']*' 
simple_op (","|";"|"("|")"|"="|"<"|">"|"*"|".")
op_neq "<>"
op_leq "<="
op_geq ">="
delim [ \r\n]
whitespaces {delim}+
%%  
"DATABASE"	{ return P_DATABASE; }
"database"	{ return P_DATABASE; }
"DROP"		{ return P_DROP; }
"drop"		{ return P_DROP; }
"INSERT"	{ return P_INSERT; }
"insert"	{ return P_INSERT; }
"UPDATE"	{ return P_UPDATE; }
"update"	{ return P_UPDATE; }
"DESC"		{ return P_DESC; }
"desc"		{ return P_DESC; }
"DATABASES"	{ return P_DATABASES; }
"databases"	{ return P_DATABASES; }
"USE"		{ return P_USE; }
"use"		{ return P_USE; }
"INTO"		{ return P_INTO; }
"into"		{ return P_INTO; }
"SET"		{ return P_SET; }
"set"		{ return P_SET; }
"INDEX"		{ return P_INDEX; }
"index"		{ return P_INDEX; }
"TABLE"		{ return P_TABLE; }
"table"		{ return P_TABLE; }
"PRIMARY"	{ return P_PRIMARY; }
"primary"	{ return P_PRIMARY; }
"VALUES"	{ return P_VALUES; }
"values"	{ return P_VALUES; }
"SELECT"	{ return P_SELECT; }
"select"	{ return P_SELECT; }
"AND"		{ return P_AND; }
"and"		{ return P_AND; }
"TABLES"	{ return P_TABLES; }
"tables"	{ return P_TABLES; }
"KEY"		{ return P_KEY; }
"key"		{ return P_KEY; }
"DELETE"	{ return P_DELETE; }
"delete"	{ return P_DELETE; }
"IS"		{ return P_IS; }
"is"		{ return P_IS; }
"SHOW"		{ return P_SHOW; }
"show"		{ return P_SHOW; }
"NOT"		{ return P_NOT; }
"not"		{ return P_NOT; }
"FROM"		{ return P_FROM; }
"from"		{ return P_FROM; }
"INT"		{ return P_INT; }
"int"		{ return P_INT; }
"CREATE"	{ return P_CREATE; }
"create"	{ return P_CREATE; }
"NULL"		{ return P_NULL; }
"null"		{ return P_NULL; }
"WHERE"		{ return P_WHERE; }
"where"		{ return P_WHERE; }
"VARCHAR"	{ return P_VARCHAR; }
"varchar"	{ return P_VARCHAR; }

{int}		{ sscanf(yytext, "%d", &yylval.val);
		  yylval.type = VALUE_INT; 
		  return VALUE_INT;
		}

{string}	{
		  yylval.str = string(yytext).substr(1, yyleng - 2);
		  yylval.type = VALUE_STRING;
		  return VALUE_STRING;
		}

{identifier} { yylval.str = string(yytext);
		  yylval.type = IDENTIFIER;
		  return IDENTIFIER;
		}

{simple_op} { return *yytext; }
{op_neq}    { return OP_NEQ; }
{op_leq}    { return OP_LEQ; }
{op_geq}    { return OP_GEQ; }
{whitespaces} { /* do nothing */ }


. {printf(".Error\n");exit(1);}    
%%
