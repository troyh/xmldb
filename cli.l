%{
#include <stdio.h>
#include "cli.tab.h"

char linebuf[1024];
unsigned int lineno=0;
unsigned int tokenpos=0;

#undef YY_INPUT
#define YY_INPUT(b,r,ms) (r=my_yyinput(b,ms))
%}


%%
new		{ tokenpos+=yyleng; return NEW_TOK; }
put 	{ tokenpos+=yyleng; return PUT_TOK; }
get 	{ tokenpos+=yyleng; return GET_TOK; }
unput 	{ tokenpos+=yyleng; return UNPUT_TOK; }
show 	{ tokenpos+=yyleng; return SHOW_TOK; }

int8|int16|int32|int64|uint8|uint16|uint32|uint64|double|char8|string	{
																	yylval.string=strdup(yytext);
																	tokenpos+=yyleng;
																	return INDEX_TYPE;
																}


-[0-9]+		{
				yylval.string=strdup(yytext);
				tokenpos+=yyleng;
				return NEG_NUMBER;
			}
[0-9]+		{
				yylval.string=strdup(yytext);
				tokenpos+=yyleng;
				return POS_NUMBER;
			}
[a-zA-Z0-9_]+	{
					yylval.string=strdup(yytext);
					tokenpos+=yyleng;
					return STRING;
				}
[ \t]+		{
				tokenpos+=yyleng;
				/* ignore */
			}
\n.*		{
				strncpy(linebuf,yytext+1,sizeof(linebuf));
				lineno++;
				tokenpos=0;
				yyless(1);
				/* ignore */
			}
.			{
				tokenpos+=yyleng;
				return (int)yytext[0];
			}
%%
