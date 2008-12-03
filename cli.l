%{
#include <stdio.h>
#include "cli.tab.h"

#undef YY_INPUT
#define YY_INPUT(b,r,ms) (r=my_yyinput(b,ms))
%}


%%
new	return NEW_TOK;
put return PUT_TOK;
get return GET_TOK;
unput return UNPUT_TOK;
show return SHOW_TOK;

int8|int16|int32|int64|uint8|uint16|uint32|uint64|double|char8	yylval.string=strdup(yytext);return INDEX_TYPE;


-?[0-9]+		yylval.number=strtol(yytext,0,10);return NUMBER;
[a-zA-Z0-9_]+	yylval.string=strdup(yytext);return STRING;
[ \t\n]+		/* ignore */
.				return (int)yytext[0];
%%
