#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///// String /////

typedef struct {
	char* text;
} string;

void string_init(string*);
void string_free(string*);
char* string_get(string*);
void string_set(string*,char*);
void string_add(string*,char*);
void string_push(string*,char);

void string_init(string* s)
{
	s->text = malloc(1);
	s->text[0] = 0;
}

void string_free(string* s) { free(s->text); }

char* string_get(string* s) { return s->text; }

void string_set(string* s,char* text)
{
	s->text = realloc(s->text, strlen(text) + 1);
	strcpy(s->text, text);
	s->text[strlen(text)] = 0;
}

void string_add(string* s,char* text)
{
	s->text = realloc(s->text, strlen(s->text) + strlen(text) + 1);
	strcat(s->text, text);
	s->text[strlen(s->text) + strlen(text)] = 0;
}

void string_push(string* s,char c)
{
	int pos = strlen(s->text);
	s->text = realloc(s->text, strlen(s->text) + 2);
	s->text[pos] = c;
	s->text[pos+1] = 0;
}

///// Lexer /////

typedef struct {
	string type;
	string lexeme;
} Token;

char* binops[] = {
	"+","-","*","/","%","==","<=",">=","<",">","!=","&","|"
};

char* punc = "{}();,";

char* keywords[] = {
	"func","if","else"
};

void scan(char** buffer, Token* tok)
{
	while (**buffer && isspace(**buffer)) { (*buffer)++; }

	if ((**buffer) == 0) { string_set(&(tok->type), "END"); return; }

	for (int i=0;i<sizeof(binops)/sizeof(char*);i++)
	{
		if (strncmp(*buffer, binops[i], strlen(binops[i])) == 0)
		{
			string_set(&(tok->type), "BINOP");
			string_set(&(tok->lexeme), binops[i]);
			return;
		}
	}

	if (strchr(punc, **buffer))
	{
		string_set(&(tok->type), "PUNC");
		string_set(&(tok->lexeme), "");
		string_push(&(tok->lexeme), **buffer);
		(*buffer)++;
		return;
	}

	if (isdigit(**buffer))
	{
		string num;
		string_init(&num);	
		while (isdigit(**buffer)) { string_push(&num, **buffer); (*buffer)++; }
		string_set(&(tok->type), "NUM");
		string_set(&(tok->lexeme), string_get(&num));
		string_free(&num);
		return;
	}

	if (**buffer == '\'')
	{
		string str;
		string_init(&str);
		(*buffer)++;
		while (**buffer != '\'') { string_push(&str, **buffer); (*buffer)++; }
		(*buffer)++;
		string_set(&(tok->type), "STR");
		string_set(&(tok->lexeme), string_get(&str));
		string_free(&str);
		return;
	}
	
	if (**buffer == '\"')
	{
		string str;
		string_init(&str);
		(*buffer)++;
		while (**buffer != '\"') { string_push(&str, **buffer); (*buffer)++; }
		(*buffer)++;
		string_set(&(tok->type), "STR");
		string_set(&(tok->lexeme), string_get(&str));
		string_free(&str);
		return;
	}

	if (isalpha(**buffer) || **buffer == '_')
	{
		
		string word;
		string_init(&word);
		while (isalnum(**buffer) || **buffer == '_') { string_push(&word, **buffer); (*buffer)++; }
		string_set(&(tok->type), "VAR");
		string_set(&(tok->lexeme), string_get(&word));
		if (strcmp(string_get(&word), "true") == 0 || strcmp(string_get(&word), "false") == 0) { string_set(&(tok->type), "BOOL"); }
		else
		{
			for (int i=0;i<sizeof(keywords)/sizeof(char*);i++)
			{
				if (strcmp(keywords[i], string_get(&word)) == 0)
				{
					string_set(&(tok->type), "KEYWORD");
					break;
				}
			}
		}
		string_free(&word);
		return;
	}
}

///// AST /////

typedef enum {
	T_NUM,
	T_STR,
	T_BOOL,
	T_VAR,
	T_BRACKETS,
	T_IF,
	T_IF_ELSE,
	T_BINARY
} nodetype;

typedef struct {
	nodetype type;
} node;

typedef struct {
	nodetype type;
	int value;
} num_node;

typedef struct {
	nodetype type;
	char* value;
} str_node;

typedef struct {
	nodetype type;
	bool value;
} bool_node;

typedef struct {
	nodetype type;
	char* name;
} var_node;

typedef struct {
	nodetype type;
	node** body;
} brackets_node;

typedef struct {
	nodetype type;
	node* cond;
	node* body;
} if_node;

typedef struct {
	nodetype type;
	node* cond;
	node* t_cl;
	node* f_cl;
} if_else_node;

typedef struct {
	nodetype type;
	node* left;
	node* right;
	char* op;
} binary_node;

///// Parser /////

int prec(char* op)
{
	if (op == "=") { return 1; }
	if (op == "|") { return 2; }
	if (op == "&") { return 3; }
	if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") { return 4; }
	if (op == "+" || op == "-") { return 5; }
	if (op == "*" || op == "/" || op == "%") { return 6; }
	fprintf(stderr, "Error: undefined binary operator: %s.\n", op);
	exit(1);
}

node* parseIf(char* buffer)
{
	node* cond = parse(&buffer);
	node* t_cl = parse(&buffer);
	if_node* n = { T_IF, cond, t_cl };
	return (node*)n;
}

node* parseBrackets(char* buffer)
{
	Token t;
	node** body = malloc(0);
	node* n;
	int size = 0;
	while (1)
	{
		if (strcmp(string_get(&(t.lexeme)), "}") == 0) { break; }
		n = parse(buffer);
		
	}
}

node* parseParentheses(char* buffer)
{

}

node* atom(char* buffer)
{
	Token t;
	scan(&buffer, &t);
	char* type = string_get(&(t.type));
	char* lexeme = string_get(&(t.lexeme));

	if (strcmp(type,"NUM") == 0) { num_node* n = { T_NUM, atoi(lexeme) }; return (node*)n; }
	if (strcmp(type,"STR") == 0) { str_node* n = { T_STR, lexeme }; return (node*)n; }
	if (strcmp(type,"BOOL") == 0) { bool_node* n = { T_BOOL, lexeme == "true" }; return (node*)n; }
	if (strcmp(type,"KEYWORD") == 0)
	{
		if (strcmp(lexeme,"if") == 0) { return parseIf(buffer); }
	}
	if (strcmp(type,"PUNC") == 0)
	{
		if (strcmp(lexeme,"{") == 0) { return parseBrackets(buffer); }
		if (strcmp(lexeme,"(") == 0) { return parseParentheses(buffer); }
	}
	fprintf(stderr, "Error: cannot parse token: %s.\n", lexeme);
	exit(1);
}

node* binary(node* left, int lastPrec, char* buffer)
{
	Token t;
	scan(&buffer, &t);
	if (strcmp(string_get(&(t.type)), "BINOP") == 0)
	{
		if (prec(string_get(&(t.type))) > lastPrec)
		{
			binary_node* n = { T_BINARY, left, binary(atom(buffer), prec(string_get(&(t.type))), buffer), string_get(&(t.lexeme)) };
			return binary((node*)(n), lastPrec, buffer);
		}
	}
	return left;
}

void parse(char* buffer)
{
	return binary(atom(buffer), 0, buffer);
}

///// Main program /////

int main(int argc, char** argv)
{
	if (argc < 2) { fprintf(stderr, "Error: need a filename.\n"); return 1; }
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

		parse(buffer);
	
	} else { fprintf(stderr, "Error: cannot open file: %s.\n", argv[1]); return 1; }
	return 0;
}
