%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ouzo_new_index(const char* name, const char* type, size_t capacity);


void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
int yywrap()
{
        return 1;
} 
  
typedef struct _NUMBER_LIST
{
	int list[10];
	size_t count;
	struct _NUMBER_LIST* next;
} NUMBER_LIST;

typedef struct _STRING_LIST
{
	char* list[10];
	size_t count;
	struct _STRING_LIST* next;
} STRING_LIST;

typedef struct _KEY_DOCID_LIST
{
	char* key;
	NUMBER_LIST* docids;
	struct _KEY_DOCID_LIST* next;
} KEY_DOCID_LIST;


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

commands: command
		| command ';'
		| commands command
		| commands command ';'
		;
			
command:   NEW_TOK   name index_type capacity 	{ ouzo_new_index($2,$3,$4); }
		 | SHOW_TOK name 						{ printf("SHOW:%s\n",$2); }
         | PUT_TOK   name key_docid_list_set
		{ 
			KEY_DOCID_LIST* list=(KEY_DOCID_LIST*)$3;
			while (list)
			{
				printf("PUT:%s:%s:",$2,list->key);
				NUMBER_LIST* p=list->docids;
				for (; p; p=p->next)
				{
					size_t i;
					for (i=0;i<p->count;++i)
					{
						printf("%c%d",(i?',':' '),p->list[i]);
					}
				}
				printf("\n");
				
				KEY_DOCID_LIST* freelist=list;
				
				list=list->next;
				
				free(freelist);
			}
		}
		 | GET_TOK   name key_list				
		{ 
			STRING_LIST* list=(STRING_LIST*)$3;
			while  (list)
			{
				size_t i;
				for (i=0;i<list->count;++i)
				{
					printf("GET:%s:%s\n",$2,list->list[i]);
					free(list->list[i]);
				}

				STRING_LIST* freelist=list;
				list=list->next;
				
				free(freelist);
			}
		}
		 | UNPUT_TOK name key_docid_list_set	
		{ 
			KEY_DOCID_LIST* list=(KEY_DOCID_LIST*)$3;
			while (list)
			{
				printf("UNPUT:%s:%s:",$2,list->key);
				NUMBER_LIST* p=list->docids;
				for (; p; p=p->next)
				{
					size_t i;
					for (i=0;i<p->count;++i)
					{
						printf("%c%d",(i?',':' '),p->list[i]);
					}
				}
				printf("\n");
			
				KEY_DOCID_LIST* freelist=list;
			
				list=list->next;
			
				free(freelist);
			}
		}

key_list: key 
		{
			STRING_LIST* new=(STRING_LIST*)calloc(1,sizeof(STRING_LIST));
			if (!new)
			{
				/* TODO: fail gracefully */
			}
			
			new->list[new->count++]=$1;
			$$=new;
		}
		| key_list key
		{
			STRING_LIST* p=$1;
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
			
			p->list[p->count++]=$2;
			$$=p;
		}
		;

key_docid_list_set: key_docid_list
		{
			$$=$1;
		}
		| key_docid_list_set key_docid_list
		{
			/* Find end of chain */
			KEY_DOCID_LIST* p=$1;
			for (; p->next; p=p->next)
			{
			}
			
			p->next=(KEY_DOCID_LIST*)$2;
			$$=$1;
		}
		
key_docid_list: '(' key ':' docid_list ')'
	{
		KEY_DOCID_LIST* p=(KEY_DOCID_LIST*)calloc(1,sizeof(KEY_DOCID_LIST));
		if (!p)
		{
			/* TODO: fail gracefully */
		}
		
		p->key=$2;
		p->docids=(NUMBER_LIST*)$4;
		
		$$=p;
	}

docid_list: NUMBER
		{
			NUMBER_LIST* number_list;
			number_list=(NUMBER_LIST*)calloc(1,sizeof(NUMBER_LIST));
			number_list->list[number_list->count++]=$1;
			$$=number_list;
		}
		| docid_list NUMBER 
		{ 
			// Find the next empty slot
			NUMBER_LIST* p=(NUMBER_LIST*)$1;
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

			p->list[p->count++]=$2;
			$$=$1;
		}
		;


index_type: INDEX_TYPE { $$=$1; }

capacity: NUMBER { $$=$1; }

key: STRING { $$=$1; }

name: STRING { $$=$1; }

%%		 
/*		
new <name> <type> <capacity>
put <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...]),,,]
get <name> <key> [<key>...]
unput <name> (<key>:<docid> [<docid>...]) [(<key>:<docid> [<docid>...])...]
show index <name> [<name>]

*/
