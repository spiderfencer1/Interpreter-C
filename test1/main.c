#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __DEBUG__ 1

char* string_add(char* a, char* b)
{
	char* str = malloc(strlen(a) + strlen(b) + 1);
	strcat(str,a);
	strcat(str,b);
	return str;
}

///// Vector /////

typedef struct
{
	int length;
	int pos;
	void** data;
} vector;

#define VECTOR_START_SIZE 10

void vector_init(vector*);
void vector_resize(vector*,int);
void vector_add(vector*,void*);
void vector_extend(vector*,vector*);
int vector_len(vector*);
void* vector_get(vector*,int);
void vector_free(vector*);

void vector_init(vector* v)
{
	v->length = VECTOR_START_SIZE;
	v->pos = 0;
	v->data = malloc(sizeof(void*) * v->length);
}

void vector_resize(vector* v, int length)
{
	v->length = length;
	v->data = realloc(v->data, sizeof(void*) * length);
}

void vector_add(vector* v, void* elem)
{
	if (v->pos >= v->length) { vector_resize(v, v->length*2); }
	v->data[v->pos++] = elem;
}

void vector_extend(vector* v, vector* values)
{
	for (int i=0;i<vector_len(values);i++)
	{
		vector_add(v, vector_get(values, i));
	}
}

int vector_len(vector* v) { return v->pos; }

void* vector_get(vector* v, int index)
{
	if (index >= 0 && index < v->pos) { return v->data[index]; }
	return NULL;
}

void vector_free(vector* v) { free(v->data); }

///// Lexer /////

enum tokentype { BINOP, PUNC, NUM, STR, BOOL, KEYWORD, VAR, END };

typedef enum tokentype TokenType;

const char* binops[] = {
	"+","-","*","/","%","<=",">=","==","!=","<",">","=","&","|"
};
char* punc = "[](){};";
char* keywords[] = {
	"func","if","else"
};

TokenType type;
char* lexeme;
int lineno;

void scan(char* src)
{
	if (*src == '\0') { type = END; return; }

	while (*src && isspace(*src)) { src++; }
	if (*src && *src == '\n') { lineno++; }

	for (int i=0;i<sizeof(binops)/sizeof(binops[0]);i++)
	{
		if (strncmp(src, binops[i], strlen(binops[i])) == 0)
		{
			strcpy(lexeme, "");
			src += strlen(binops[i]);
			type = BINOP;
			lexeme = realloc(lexeme, strlen(binops[i]) + 1);
			strcpy(lexeme, binops[i]);
			return;
		}
	}

	if (strchr(punc,*src))
	{
		strcpy(lexeme, "");
		type = PUNC;
		lexeme = realloc(lexeme, 2);
		strncpy(lexeme, src, 1);
		return;
	}

	if (isdigit(*src))
	{
		memset(lexeme, 0, strlen(lexeme));
		type = NUM;
		while (*src && isdigit(*src))
		{
			lexeme = realloc(lexeme, strlen(lexeme) + 2);
			strncat(lexeme, src, 1);
			printf("%s\n", lexeme);
			src++;
		}
		return;
	}

	if (*src == '\'' || *src == '\"')
	{
		strcpy(lexeme, "");
		char end = *src;
		type = STR;
		while (*src && *src != end)
		{
			lexeme = realloc(lexeme, strlen(lexeme) + 2);
			strncat(lexeme, src, 1);
			src++;
		}
		src++;
		return; 
	}

	if (isalpha(*src) || *src == '_')
	{
		strcpy(lexeme, "");
		type = VAR;
		while (*src && (isalnum(*src) || *src == '_'))
		{
			lexeme = realloc(lexeme, strlen(lexeme) + 2);
			strncat(lexeme, src, 1);
			src++;
		}
		if (strcmp(lexeme,"true") == 0 || strcmp(lexeme,"false") == 0) { type = BOOL; }
		for (int i=0;i<sizeof(keywords)/sizeof(keywords[0]);i++) { if (strcmp(keywords[i],lexeme) == 0) { type = KEYWORD; } }
		return;
	}

	if (*src == '#')
	{
		while (*src && *src != '\n') { src++; }
		src++;
	}

	src++;
}

///// Main program /////

int main(int argc, char** argv)
{
	if (argc < 2) { printf("Error: need a filename.\n"); return 1; }
	FILE* fp = fopen(argv[1], "r");
	long length;
	char* buffer;
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = malloc(length);
		fread(buffer, 1, length, fp);
		fclose(fp);

		lexeme = malloc(1);

		do
		{
			scan(buffer);
			printf("%d\n", type);
		} while (type != END);

		free(buffer);
	}
	else { printf("Error: cannot open file: %s.\n", argv[1]); return 1; }
	return 0;
}
