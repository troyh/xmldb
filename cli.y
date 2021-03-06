%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

STRING_LIST* start_string_list(char* s)
{
	STRING_LIST* new=(STRING_LIST*)calloc(1,sizeof(STRING_LIST));
	if (!new)
	{
		/* TODO: fail gracefully */
	}
	
	new->list[new->count++]=s;
	return new;
}

STRING_LIST* append_string_list(STRING_LIST* p, char* s)
{
	STRING_LIST* origp=p;
	
	while (p->next && p->count>=(sizeof(p->list)/sizeof(p->list[0])))
	{
		p=p->next;
	}
	
	if (p->count<(sizeof(p->list)/sizeof(p->list[0])))
	{
		/* Do nothing, there's room left in this block */
	}
	else 
	{
		p->next=(STRING_LIST*)calloc(1,sizeof(STRING_LIST));
		p=p->next;
	}
	
	p->list[p->count++]=s;
	return origp;
}

KEY_DOCID_LIST* connect_number_list(KEY_DOCID_LIST* p, KEY_DOCID_LIST* list)
{
	KEY_DOCID_LIST* origp=p;
	
	/* Find end of chain */
	for (; p->next; p=p->next)
	{
	}
	
	p->next=list;
	return origp;
}

KEY_DOCID_LIST* make_key_docid_list(char* key, NUMBER_LIST* nl)
{
	KEY_DOCID_LIST* p=(KEY_DOCID_LIST*)calloc(1,sizeof(KEY_DOCID_LIST));
	if (!p)
	{
		/* TODO: fail gracefully */
	}
	
	p->key=key;
	p->docids=nl;
	
	return p;
}

NUMBER_LIST* start_number_list(int n)
{
	NUMBER_LIST* number_list=(NUMBER_LIST*)calloc(1,sizeof(NUMBER_LIST));
	number_list->list[number_list->count++]=n;
	return number_list;
}

NUMBER_LIST* append_number_list(NUMBER_LIST* p, int n)
{ 
	NUMBER_LIST* origp=p;
	
	// Find the next empty slot
	while (p && p->next && (p->count>=(sizeof(p->list)/sizeof(p->list[0]))))
	{
		p=p->next;
	}
	
	if (!p)
	{
		/* Shouldn't happen, it was alloc'd in the previous action */
	}
	else if (p->count<(sizeof(p->list)/sizeof(p->list[0])))
	{
		/* Do nothing, there's room in this block */
	}
	else if (!p->next)
	{
		p->next=(NUMBER_LIST*)calloc(1,sizeof(NUMBER_LIST));
		p=p->next;
	}
	
	if (!p)
	{
		/* ACK! Out of memory. Do something good. */
	}

	p->list[p->count++]=n;

	return origp;
}

char* make_string_from_unsigned_number(unsigned long n)
{
	char buf[64];
	sprintf(buf,"%lu",n);
	return strdup(buf);
}

char* make_string_from_signed_number(long n)
{
	char buf[64];
	sprintf(buf,"%ld",n);
	return strdup(buf);
}

%}


%union
{
	unsigned long unsigned_number;
	long signed_number;
	char* string;
	void* vptr;
}

%token <string> POS_NUMBER 
%token <string> NEG_NUMBER 
%token <string> FLOAT_NUMBER
%token <string> STRING 
%token <string> NEW_TOK 
%token <string> SHOW_TOK 
%token <string> INFO_TOK 
%token <string> PUT_TOK
%token <string> PUTDOC_TOK
%token <string> DELDOC_TOK
%token <string> GET_TOK
%token <string> UNPUT_TOK
%token <string> QUIT_TOK
%token <string> QUERY_TOK
%token <string> AND_TOK
%token <string> OR_TOK
%token <string> INDEX_TYPE

%type <string> key 
%type <string> name 
%type <string> index_type 
%type <string> filename
%type <unsigned_number> number 
%type <unsigned_number> capacity 
%type <vptr> docid_list 
%type <vptr> key_docid_list
%type <vptr> key_docid_list_set
%type <vptr> key_list
/*
%type <vptr> query_expression
%type <vptr> query_clause
%type <vptr> equality_expression
%type <vptr> equality_clause
%type <string> query_key
%type <string> docbase_name
%type <unsigned_number> boolean_oper
%type <unsigned_number> equality_oper
%type <string> equality_value
*/

%%

/*		
new <name> <type> <capacity>
put <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...]),,,]
get <name> <key> [<key>...]
unput <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...])...]
show index <name> [<name>]

*/

commands:			  /* empty */
					| commands ';'
					| commands command ';'								

			
command:   			   NEW_TOK    name index_type capacity 				{ ouzo_new_index($2,$3,$4); }
					 | SHOW_TOK   name 									{ ouzo_show_index($2); }
			         | PUT_TOK    name key_docid_list_set				{ ouzo_index_put($2,$3); }
			         | PUTDOC_TOK filename								{ ouzo_docbase_put($2); }
			         | DELDOC_TOK filename								{ ouzo_docbase_del($2); }
					 | UNPUT_TOK  name key_docid_list_set				{ ouzo_index_unput($2,$3); }
					 | GET_TOK    name key_list							{ ouzo_index_get($2,$3); }
					 | QUERY_TOK  										{ ouzo_query(); }
/*					 | QUERY_TOK  query_expression						{ ouzo_query($2); }*/
					 | INFO_TOK											{ ouzo_info(); }
					 | QUIT_TOK											{ cli_quit(); }
/*					
query_expression:     query_clause										{ $$=start_query_expression($1); }
					| query_expression boolean_oper query_clause		{ $$=append_query_expression($1,$2,$3); }

query_clause:		  docbase_name ':' equality_expression				{ $$=make_query_clause($1,$3); }

equality_expression:  equality_clause
					| equality_expression boolean_oper equality_clause

equality_clause:      query_key equality_oper equality_value			{ $$=make_equality_expression($1,$2,$3); }
					| '(' query_key equality_oper equality_value ')'  	{ $$=make_equality_expression($2,$3,$4); }

query_key:			  docbase_name '.' index_name

docbase_name:		  STRING

index_name:		      STRING

boolean_oper:         AND_TOK 		{ $$=1; }
					| OR_TOK		{ $$=2; }

equality_oper:       '<'			{ $$=1; }
 					| '>' 			{ $$=2; }
					| '=' 			{ $$=3; }
					| '<' '=' 		{ $$=4; }
					| '>' '=' 		{ $$=5; }
					| '!' '='		{ $$=6; }
					
equality_value:		  POS_NUMBER		{ $$=$1; }
					| NEG_NUMBER		{ $$=$1; }
					| '"' STRING '"' 	{ $$=$2; }
*/

key_list: 	  		  key 												{ $$=start_string_list($1); }
					| key_list key										{ $$=append_string_list($1,$2); }

key_docid_list_set:   key_docid_list									{ $$=$1; }
					| key_docid_list_set key_docid_list					{ $$=connect_number_list($1,$2); }
		
key_docid_list: 	'(' key ':' docid_list ')'							{ $$=make_key_docid_list($2,$4); }

docid_list: 		  number											{ $$=start_number_list($1); }
					| docid_list number 								{ $$=append_number_list($1,$2); }

index_type: 		INDEX_TYPE 											{ $$=$1; }
capacity: 			POS_NUMBER											{ $$=strtoul($1,0,10); }
key: 				STRING 												{ $$=$1; }
 					| POS_NUMBER										{ $$=$1; }
 					| NEG_NUMBER										{ $$=$1; }
 					| FLOAT_NUMBER										{ $$=$1; }

filename:           STRING												{ $$=$1; }

number: 			POS_NUMBER											{ $<unsigned_number>$=strtoul($1,0,10); }
					| NEG_NUMBER 										{ $<signed_number>$=strtol($1,0,10); }

name: 				STRING 												{ $$=$1; }

%%		 

