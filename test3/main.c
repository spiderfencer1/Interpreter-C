#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///// Utilities /////

char* append(char* a, char b)
{
	char* c = malloc(strlen(a) + 2);
	strcpy(c,a);
	c[strlen(a)] = b;
	c[strlen(a) + 1] = 0;
	return c;
}

///// Vector /////

typedef struct
{
	void** elements;
} vector;

void vector_init(vector*);
void vector_free(vector*);
void vector_push(vector*,void*);
void* vector_get(vector*,int);
int vector_size(vector*);

void vector_init(vector* v) { v->elements = malloc(0); }

void vector_free(vector* v) { free(v->elements); }

void vector_push(vector* v,void* elem)
{
	v->elements = realloc(v->elements, vector_size(v) + 1);
	v->elements[vector_size(v) - 1] = elem;
}

void* vector_get(vector* v,int i)
{
	if (i >= 0 && i < vector_size(v)) { return v->elements[i]; }
	return NULL;
}

int vector_size(vector* v) { return sizeof(v->elements) / sizeof(void*); }

///// Lexer /////

typedef enum
{
	T_BINOP,
	T_PUNC,
	T_NUM,
	T_STR,
	T_BOOL,
	T_KEYWORD,
	T_VAR
} tokentype;

typedef struct
{
	tokentype type;
	char* lexeme;
} token;

vector lex(char* buffer)
{
	vector tokens;
	vector_init(&tokens);
	int pos = 0;

	char* binops[] = { "+","-","*","/","%","==","!=","<=",">=","<",">","|","&" }; 
	char* punc = "{}();,";
	char* keywords[] = { "func","if","else" };

	char op[3];

	while (pos < strlen(buffer))
	{
		for (int i=0;i<sizeof(binops)/sizeof(char*);i++)
		{
			strncpy(op, buffer+pos, strlen(binops[i]));
			if (strcmp(op, binops[i]) == 0) { pos += strlen(op); token t = { T_BINOP, binops[i] }; vector_push(&tokens, &t); }
		}

		if (strchr(punc, buffer[pos])) { pos++; char p[2]; p[0] = buffer[pos]; p[1] = 0; token t = { T_PUNC, p }; vector_push(&tokens, &t); }
		if (isdigit(buffer[pos]))
		{
			int num = buffer[pos++] - '0';
			while (pos < strlen(buffer) && isdigit(buffer[pos]))
			{
				num = num * 10 + (buffer[pos++] - '0');
			}
			char snum[(int)(floor(log10(abs(num)))) + 2];
			sprintf(snum, "%d", num);
			token t = { T_NUM, snum };
			vector_push(&tokens, &t);
		}

		if (buffer[pos] == '\'')
		{
			char* str = malloc(1);
			pos++;
			while (buffer[pos] != '\'') { strcpy(str,append(str,buffer[pos++])); }
			token t = { T_STR, str };
			vector_push(&tokens, &t);
		}
		
		if (buffer[pos] == '\"')
		{
			char* str = malloc(1);
			pos++;
			while (buffer[pos] != '\"') { strcpy(str,append(str,buffer[pos++])); }
			token t = { T_STR, str };
			vector_push(&tokens, &t);
		}

		if (isalpha(buffer[pos]))
		{
			char* word = malloc(sizeof(char));
			word[0] = 0;
			while (isalnum(buffer[pos]) || buffer[pos] == '_') { printf("%s\n",word); word = realloc(word, strlen(word) + 1); word[strlen(word)] = buffer[pos++]; }	
			token t = { T_VAR, word };
			if (strcmp(word,"true") == 0 || strcmp(word,"false") == 0) { t.type = T_BOOL; }
			for (int i=0;i<sizeof(keywords)/sizeof(keywords[0]);i++) { if (strcmp(word,keywords[i]) == 0) { t.type = T_KEYWORD; break; } }
			vector_push(&tokens, &t);
		}

		if (buffer[pos] == '#')
		{
			while (buffer[pos] != '\n') { pos++; }
		}

		pos++;
	}

	return tokens;
}

///// Main program. /////

int main(int argc, char** argv)
{
	if (argc < 2) { fprintf(stderr, "Error: cannot open file.\n"); return 1; }
	FILE* fp = fopen(argv[1], "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = malloc(length + 1);
		fread(buffer, 1, length, fp);
		fclose(fp);

		buffer[length] = 0;

		vector tokens = lex(buffer);

		for (int i=0;i<vector_size(&tokens);i++)
		{
			token* t = (token*)(vector_get(&tokens, i));
			printf("Token: %s\n", t->lexeme);
		}

		free(buffer);

	} else { fprintf(stderr, "Error: cannot open file: %s.\n", argv[1]); return 1; }

}
