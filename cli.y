%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
int yywrap()
{
        return 1;
} 

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
	while (p->next && p->count<(sizeof(p->list)/sizeof(p->list[0])))
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
	return p;
}

KEY_DOCID_LIST* connect_number_list(KEY_DOCID_LIST* p, KEY_DOCID_LIST* list)
{
	/* Find end of chain */
	for (; p->next; p=p->next)
	{
	}
	
	p->next=list;
	return p;
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
	// Find the next empty slot
	while (p && p->next && (p->count<(sizeof(p->list)/sizeof(p->list[0]))))
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
	return p;
}


%}


%union
{
	int number;
	char* string;
	void* vptr;
}

%token <number> NUMBER 
%token <string> STRING 
%token <string> NEW_TOK 
%token <string> SHOW_TOK 
%token <string> PUT_TOK
%token <string> GET_TOK
%token <string> UNPUT_TOK
%token <string> INDEX_TYPE

%type <string> key 
%type <string> name 
%type <string> index_type 
%type <number> capacity 
%type <vptr> docid_list 
%type <vptr> key_docid_list
%type <vptr> key_docid_list_set
%type <vptr> key_list

%%

/*		
new <name> <type> <capacity>
put <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...]),,,]
get <name> <key> [<key>...]
unput <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...])...]
show index <name> [<name>]

*/

commands:			  command
					| command ';'
					| commands command
					| commands command ';'
					;
			
command:   			   NEW_TOK   name index_type capacity 				{ ouzo_new_index($2,$3,$4); }
					 | SHOW_TOK name 									{ ouzo_show_index($2); }
			         | PUT_TOK   name key_docid_list_set				{ ouzo_index_put($2,$3); }
					 | UNPUT_TOK name key_docid_list_set				{ ouzo_index_unput($2,$3); }
					 | GET_TOK   name key_list							{ ouzo_index_get($2,$3); }

key_list: 	  		  key 												{ $$=start_string_list($1); }
					| key_list key										{ $$=append_string_list($1,$2); }

key_docid_list_set:   key_docid_list									{ $$=$1; }
					| key_docid_list_set key_docid_list					{ $$=connect_number_list($1,$2); }
		
key_docid_list: 	'(' key ':' docid_list ')'							{ $$=make_key_docid_list($2,$4); }

docid_list: 		  NUMBER											{ $$=start_number_list($1); }
					| docid_list NUMBER 								{ $$=append_number_list($1,$2); }


index_type: 		INDEX_TYPE 											{ $$=$1; }
capacity: 			NUMBER 												{ $$=$1; }
key: 				STRING 												{ $$=$1; }
name: 				STRING 												{ $$=$1; }

%%		 

