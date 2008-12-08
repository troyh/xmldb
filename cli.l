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
quit|exit 	{ tokenpos+=yyleng; return QUIT_TOK; }

int8|int16|int32|int64|uint8|uint16|uint32|uint64|double|char8|string|date|time	{
																	yylval.string=strdup(yytext);
																	tokenpos+=yyleng;
																	return INDEX_TYPE;
																}

\n.*		{
				strncpy(linebuf,yytext+1,sizeof(linebuf));
				lineno++;
				printf("Line #%d\n",lineno);
				tokenpos=0;
				yyless(1);
				/* ignore */
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
-?[0-9]+[0-9\.]+e\+[0-9]+ 	{	/* e syntax */
								yylval.string=strdup(yytext);
								tokenpos+=yyleng;
								return FLOAT_NUMBER;
							}
-?[0-9]+\.[0-9]+ 			{
								yylval.string=strdup(yytext);
								tokenpos+=yyleng;
								return FLOAT_NUMBER;
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
.			{
				tokenpos+=yyleng;
				return (int)yytext[0];
			}
%%
