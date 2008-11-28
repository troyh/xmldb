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


STRING_LIST* start_string_list(char* s);
STRING_LIST* append_string_list(STRING_LIST* p, char* s);
KEY_DOCID_LIST* connect_number_list(KEY_DOCID_LIST* p, KEY_DOCID_LIST* list);
KEY_DOCID_LIST* make_key_docid_list(char* key, NUMBER_LIST* nl);
NUMBER_LIST* start_number_list(int n);
NUMBER_LIST* append_number_list(NUMBER_LIST* p, int n);
