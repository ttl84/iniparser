#include "iniparser.h"
#include "container/hashsetv.h"
#include "varstr.h"
#include "stringop.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ini ini char reader
struct char_stream{
	int c;
	int escaped;
	unsigned row, col;
	FILE * fid;
	int end;
};
static struct char_stream * char_stream_new(FILE * fid)
{
	struct char_stream * cs = calloc(1, sizeof *cs);
	if(cs != NULL && fid != NULL)
	{
		cs->c = -1;
		cs->escaped = 0;
		cs->row = 1;
		cs->col = 0;
		cs->end = feof(fid);
		cs->fid = fid;
	}
	else
	{
		free(cs);
		cs = NULL;
	}
	return cs;
}
void char_stream_del(struct char_stream * cs)
{
	if(cs != NULL)
	{
		cs->fid = 0;
		free(cs);
	}
}
static int next_char(struct char_stream * stream)
{
	if(!stream->end)
	{
		int escape = 0;
		do{
			int c = fgetc(stream->fid);
			if(c == EOF)
			{
				stream->c = EOF;
				stream->end = 1;
				break;
			}
			else if(c == '\n')
			{
				stream->row++;
				stream->col = 1;
			}
			else
				stream->col++;
	
			if(!escape)
			{
				if(c != '\\')
				{
					stream->c = c;
					stream->escaped = 0;
					break;
				}
				else
					escape = 1;
			}
			else
			{
				if(c == '\n')
					continue;	
				else if(c == 'n')
					stream->c = '\n';
				else if(c == 't')
					stream->c = '\t';
				else if(c == 's')
					stream->c = ' ';
				else if(c == 'b')
					stream->c = '\b';
				else if(c == 'r')
					stream->c = '\r';
				else
					stream->c = c;
				stream->escaped = 1;
				break;
			}
		}while(1);
	}
	return stream->c;
}
static int char_is_whitespace(struct char_stream * cs)
{
	return is_whitespace(cs->c) && !cs->escaped;
}
static int char_is_newline(struct char_stream * cs)
{
	return cs->c == EOF || (cs->c == '\n' && !cs->escaped);
}
static int char_is_name(struct char_stream * cs)
{
	return ((cs->c >= 'a' && cs->c <= 'z') ||
		(cs->c >= 'A' && cs->c <= 'Z') ||
		(cs->c == '_')) && !cs->escaped;
}
enum token_type{
	TOKEN_TYPE_SECTION,
	TOKEN_TYPE_NAME,
	TOKEN_TYPE_ASSIGNMENT,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_NEWLINE
};
struct token{
	enum token_type type;
	char * text;
};
static struct token * token_new(void)
{
	struct token * newtok = calloc(1, sizeof(struct token));
	if(newtok)
	{
		newtok->type = TOKEN_TYPE_NEWLINE;
		newtok->text = NULL;
	}
	return newtok;
}
static void token_del(struct token * tok)
{
	if(tok)
	{
		free(tok);
	}
}
// token stream gives one token every time it is read
struct token_stream{
	struct char_stream * cs;
	int next_state;
	int end;
};
struct token_stream * token_stream_new(FILE * fid)
{
	struct char_stream * cs = char_stream_new(fid);
	struct token_stream * ts = calloc(1, sizeof *ts);
	if(cs != NULL && ts != NULL)
	{
		ts->cs = cs;
		ts->next_state = 0;
		ts->end = 0;
	}
	else
	{
		free(ts);
		ts = NULL;
		char_stream_del(cs);
	}
	return ts;
}
static void token_stream_del(struct token_stream * ts)
{
	if(ts != NULL)
	{
		char_stream_del(ts->cs);
		ts->cs = NULL;
		free(ts);
	}
}
static struct token * next_token(struct token_stream * ts)
{
	if(ts->end)
		return NULL;
	enum states{
		READ_NEXT = 0,
		SCAN, SCAN_ERR,
		BUILD_SECTION, BUILD_SECTION1, BUILD_SECTION_END,
		BUILD_NAME, BUILD_NAME1, BUILD_NAME_END,
		BUILD_ASSIGNMENT,
		BUILD_STRING, BUILD_STRING1, BUILD_STRING_END,
		BUILD_COMMENT,
		BUILD_NEWLINE,
		BUILD_END,
		ERROR,
		SKIP_LINE,
	} state = ts->next_state;
	
	struct varstr * buf = varstr_new();
	struct char_stream * cs = ts->cs;
	struct token * tok = NULL;
	int running = 1;
	do switch(state){
	case READ_NEXT:
		next_char(cs);
		state = SCAN;
	case SCAN:
		if(!cs->escaped)
		{
			if(cs->c == ' ' || cs->c == '\t')
				state = READ_NEXT;
			else if(cs->c == '[')
				state = BUILD_SECTION;
			else if(char_is_name(cs))
				state = BUILD_NAME;
			else if(cs->c == '=')
				state = BUILD_ASSIGNMENT;
			else if(cs->c == '"')
				state = BUILD_STRING;
			else if(cs->c == '#')
				state = BUILD_COMMENT;
			else if(cs->c == '\n')
				state = BUILD_NEWLINE;
			else if(cs->c == EOF)
				state = BUILD_END;
			else
				state = SCAN_ERR;
		}
		else
			state = SCAN_ERR;
		break;
	case SCAN_ERR:
		fprintf(stderr,
			"bad token at row %d, column %d\n",
			cs->row, cs->col);
		state = SKIP_LINE;
		break;

	case BUILD_SECTION:
		// [section]
		varstr_clear(buf);
		tok = token_new();
		tok->type = TOKEN_TYPE_SECTION;
		state = BUILD_SECTION1;
	case BUILD_SECTION1:
		next_char(cs);
		if(char_is_name(cs))
			varstr_append(buf, cs->c);
		else
			state = BUILD_SECTION_END;
		break;
	case BUILD_SECTION_END:
		if(cs->c == ']' && !cs->escaped)
		{
			ts->next_state = READ_NEXT;
			tok->text = cstr_dup(varstr_view(buf));
			running = 0;
		}
		else
		{
			fprintf(stderr,
				"bad section name at row %d, column %d, "
				"expected ']'\n",
				cs->row, cs->col);
			state = ERROR;
		}
		break;
	case BUILD_NAME:
		// name
		tok = token_new();
		tok->type = TOKEN_TYPE_NAME;
		varstr_clear(buf);
		varstr_append(buf, cs->c);
		state = BUILD_NAME1;
	case BUILD_NAME1:
		next_char(cs);
		if(char_is_name(cs))
			varstr_append(buf, cs->c);
		else
			state = BUILD_NAME_END;
		break;
	case BUILD_NAME_END:
		if(cs->c == '=' && !cs->escaped)
		{
			ts->next_state = BUILD_ASSIGNMENT;
			tok->text = cstr_dup(varstr_view(buf));
			running = 0;
		}
		else if((cs->c == ' ' || cs->c == '\t') && !cs->escaped)
		{
			ts->next_state = READ_NEXT;
			tok->text = cstr_dup(varstr_view(buf));
			running = 0;
		}
		else
		{
			fprintf(stderr,
				"bad name at row %d, column %d, expected '='\n",
				cs->row, cs->col);
			state = ERROR;
		}
		break;
	case BUILD_ASSIGNMENT:
		// =
		tok = token_new();
		tok->type = TOKEN_TYPE_ASSIGNMENT;
		ts->next_state = READ_NEXT;
		running = 0;
	case BUILD_STRING:
		/* "multi
			line
				string
					with escape /"/n"
		*/
		tok = token_new();
		tok->type = TOKEN_TYPE_STRING;
		varstr_clear(buf);
		state = BUILD_STRING1;
	case BUILD_STRING1:
		next_char(cs);
		if(cs->c == EOF)
			state = BUILD_STRING_END;
		else if(cs->c == '"' && !cs->escaped)
			state = BUILD_STRING_END;
		else
			varstr_append(buf, cs->c);
		break;
	case BUILD_STRING_END:
		if(cs->c == '"' && !cs->escaped)
		{
			ts->next_state = READ_NEXT;
			tok->text = cstr_dup(varstr_view(buf));
			running = 0;
		}
		else if(cs->c == EOF)
		{
			fprintf(stderr,
				"bad string at row %d, column %d, "
				"expected '\"'\n",
				cs->row, cs->col);
			state = ERROR;
		}
		break;
	case BUILD_COMMENT:
		// # comment
		state = SKIP_LINE;
		break;
	case BUILD_NEWLINE:
		tok = token_new();
		tok->type = TOKEN_TYPE_NEWLINE;
		ts->next_state = READ_NEXT;
		running = 0;
	case BUILD_END:
		tok = token_new();
		tok->type = TOKEN_TYPE_NEWLINE;
		ts->end = 1;
		running = 0;
	case ERROR:
		token_del(tok);
		tok = NULL;
		state = SKIP_LINE;
	case SKIP_LINE:
		if(cs->c == '\n' && !cs->escaped)
			state = BUILD_NEWLINE;
		else if(cs->c == EOF)
			state = BUILD_END;
		else
			next_char(cs);
		break;
	}while(running);
	varstr_del(buf);
	return tok;
}
enum command_name{
	SET_SECTION,
	INSERT_PAIR
};
struct command{
	enum command_name type;
	char * arg1;
	char * arg2;
};
static struct command * command_new(void)
{
	return calloc(1, sizeof(struct command));
}
static void command_del(struct command * com)
{
	if(com != NULL)
	{
		com->arg1 = NULL;
		com->arg2 = NULL;
		free(com);
	}
}
struct command_stream{
	struct token_stream * ts;
	int next_state;
	int end;
};
static struct command_stream * command_stream_new(FILE * fid)
{
	struct token_stream * ts = token_stream_new(fid);
	struct command_stream * cs = calloc(1, sizeof * cs);
	if(ts != NULL && cs != NULL)
	{
		cs->ts = ts;
		cs->next_state = 0;
		cs->end = 0;
	}
	else
	{
		free(cs);
		cs = NULL;
		token_stream_del(ts);
	}
	return cs;
}
static struct command * next_command(struct command_stream * cs)
{
	enum states{
		READ_NEXT,
		SCAN,
		BUILD_SET_SECTION,
		BUILD_INSERT_PAIR,
		BUILD_END,
		ERROR,
		SKIP_LINE,
	} state = cs->next_state;
	
	struct token_stream * ts = cs->ts;
	struct token * tok = NULL;
	struct command * com = NULL;
	
	do switch(state){
	case READ_NEXT:
		tok = next_token(ts);
		state = SCAN;
	case SCAN:
		if(tok == NULL)
			state = BUILD_END;
		else if(tok->type == TOKEN_TYPE_SECTION)
			state = BUILD_SET_SECTION;
		else if(tok->type == TOKEN_TYPE_NAME)
			state = BUILD_INSERT_PAIR;
		else if(tok->type == TOKEN_TYPE_NEWLINE)
			state = READ_NEXT;
		else
			state = SKIP_LINE;
		break;
	case BUILD_SET_SECTION:
		com = command_new();
		com->type = SET_SECTION;
		com->arg1 = tok->text;
		tok->text = NULL;
		token_del(tok);
		tok = NULL;

		tok = next_token(ts);
		assert(tok != NULL);
		if(tok->type == TOKEN_TYPE_NEWLINE)
		{
			token_del(tok);
			cs->next_state = READ_NEXT;
			return com;
		}
		else
			state = ERROR;
		break;
	case BUILD_INSERT_PAIR:
		com = command_new();
		com->type = INSERT_PAIR;
		com->arg1 = tok->text;
		tok->text = NULL;
		token_del(tok);
		tok = NULL;

		tok = next_token(ts);
		assert(tok != NULL);
		if(tok->type == TOKEN_TYPE_ASSIGNMENT)
		{
			token_del(tok);
			tok = NULL;
		}
		else
		{
			state = ERROR;
			break;
		}

		tok = next_token(ts);
		assert(tok != NULL);
		if(tok->type == TOKEN_TYPE_NAME || tok->type == TOKEN_TYPE_STRING)
		{
			com->arg2 = tok->text;
			tok->text = NULL;
			token_del(tok);
			tok = NULL;
		}
		else
		{
			state = ERROR;
			break;
		}

		tok = next_token(ts);
		assert(tok != NULL);
		if(tok->type == TOKEN_TYPE_NEWLINE)
		{
			cs->next_state = READ_NEXT;
			return com;
		}
		else
		{
			state = ERROR;
		}
		break;
	case BUILD_END:
		cs->end = 1;
		cs->next_state = BUILD_END;
		return NULL;
	case ERROR:
		if(com != NULL)
		{
			free(com->arg1);
			free(com->arg2);
			command_del(com);
			com = NULL;
		}
		state = SKIP_LINE;
	case SKIP_LINE:
		if(tok->type == TOKEN_TYPE_NEWLINE)
		{
			token_del(tok);
			state = READ_NEXT;
		}
		else
		{
			token_del(tok);
			tok = next_token(ts);
		}
		break;
	}while(1);
}
struct entry{
	char * name;
	void * val;
};
static int entry_cmp(void const * a, void const * b)
{
	return strcmp(((struct entry *)a)->name, ((struct entry *)b)->name);
}
static unsigned entry_hash(void const * a)
{
	unsigned sum = 0;
	char const * s = ((struct entry *)a)->name;
	while(*s)
		sum = (sum * 31) + *s++;
	return sum;
}

// ini ini data
struct ini{
	struct hashset * symtable;
};

struct ini * ini_new(void)
{
	struct ini * ini = calloc(1, sizeof *ini);
	struct hashset * hs =
		hashset_new(sizeof(struct entry), &entry_cmp, &entry_hash);
	if(ini != NULL && hs != NULL)
	{
		ini->symtable = hs;
	}
	else
	{
		free(ini);
		ini = NULL;
		hashset_del(hs);
	}
	return ini;
}
int ini_read(struct ini * ini, FILE * fid)
{
	if(ini == NULL || fid == NULL)
		return -1;
	struct hashset * section = NULL;
	struct command_stream * cs = command_stream_new(fid);
	struct command * com = command_new();
	com->type = SET_SECTION;
	com->arg1 = calloc(1, 1);
	while(com != NULL)
	{
		if(com->type == SET_SECTION)
		{
			struct entry key = {.name = com->arg1};
			if(!hashset_get(ini->symtable, &key))
			{
				struct entry e = {
					.name = cstr_dup(com->arg1),
					.val = hashset_new(sizeof(struct entry), &entry_cmp, &entry_hash)
				};
				hashset_insert(ini->symtable, &e);
				section = e.val;
			}
			else
			{
				struct entry * section_entry =
					hashset_get(ini->symtable, com->arg1);
				section = section_entry->val;
			}
			free(com->arg1);
			free(com->arg2);
			command_del(com);
		}
		else if(com->type == INSERT_PAIR)
		{
			struct entry key = {.name = com->arg1};
			struct entry * ret = hashset_get(section, &key);
			if(ret != NULL)
			{
				struct entry old = *ret;
				hashset_remove(section, ret);
				free(old.name);
				free(old.val);
			}
			struct entry e = {
				.name = cstr_dup(com->arg1),
				.val = cstr_dup(com->arg2)
			};
			hashset_insert(section, &e);
			
			free(com->arg1);
			free(com->arg2);
			command_del(com);
		}
		com = next_command(cs);
	}
	return 0;
}
void ini_del(struct ini * ini)
{
	if(ini != NULL)
	{
		fprintf(stderr, "ini_del not implemented\n");
	}
}
char const * ini_get(struct ini const * ini,
	char * section, char * name)
{
	struct entry * ret = NULL;
	{
		struct entry key = {.name = (section != NULL ? section : "")};
		ret = hashset_get(ini->symtable, &key);
	}
	if(ret != NULL)
	{
		struct entry key = {.name = name};
		ret = hashset_get(ret->val, &key);
	}
	if(ret != NULL)
		return ret->val;
	else
		return NULL;
}

