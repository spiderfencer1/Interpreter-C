#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void p_init(void)
{
	out = fopen("a.out", "w");
	if (!out)
	{
		fprintf(stderr, "Error: cannot write to output file.\n");
		exit(1);
	}
}

void p_close(void)
{
	fclose(out);
}

void parse(char* buffer)
{
	while (1)
	{
		fprintf(out, "%s;\n", expr(buffer));
		if (type == T_END) { return; }
	}
}

char* expr(char* buffer)
{
	return binary(buffer, atom(buffer));
}

char* binary(char* buffer, char* left)
{
	l_scan(buffer);
	char* str = malloc(strlen(left));
	char* op = malloc(0);
	char* right;
	strcpy(str,left);
	while (type == T_BINOP)
	{
		op = realloc(op,strlen(lexeme));
		strcpy(op,lexeme);
		right = atom(buffer);
		str = realloc(str, strlen(str) + strlen(lexeme) + strlen(right));
		strcat(str,op);
		strcat(str,right);
		free(right);
	}
	free(op);
	return str;
}

char* atom(char* buffer)
{
	l_scan(buffer);
	char* r;
	switch (type)
	{
		case T_NUM:
		case T_BOOL:
		case T_VAR:
			r = malloc(strlen(lexeme));
			strcpy(r,lexeme);
			break;
		case T_STR:
			r = malloc(strlen(lexeme) + 2);
			strcpy(r,"\"");
			strcat(r,lexeme);
			strcat(r,"\"");
			break;
		case T_KEYWORD:
			if (strcmp(lexeme,"if") == 0) { r = parseIf(buffer); }
			break;
		case T_PUNC:
			if (strcmp(lexeme,"{") == 0) { r = parseBrackets(buffer); }
			if (strcmp(lexeme,"(") == 0) { r = parseParentheses(buffer); }
			break;
		case T_END:
			return "";
		default:
			break;
	}
	return r;
}

char* parseIf(char* buffer)
{
	char* statement = malloc(3);
	strcpy(statement,"if(");

	char* cond = expr(buffer);
	statement = realloc(statement,strlen(statement) + strlen(cond) + 1);
	strcat(statement,cond);
	strcat(statement,")");

	char* body = expr(buffer);
	long size = strlen(statement) + strlen(body);
	if (body[0] != '{') { size += 2; }
	statement = realloc(statement,size);
	if (body[0] != '{') { strcat(statement,"{"); }
	strcat(statement,body);
	if (body[0] != '{') { strcat(statement,"}"); }

	if (strcmp(lexeme,"else") == 0)
	{
		statement = realloc(statement, strlen(statement) + 4);
		strcat(statement,"else");

		char* f_cl = expr(buffer);
		long add = strlen(statement) + strlen(f_cl);
		if (f_cl[0] != '{') { add += 2; }
		statement = realloc(statement,add);
		if (f_cl[0] != '{') { strcat(statement,"{"); }
		strcat(statement,f_cl);
		if (f_cl[0] != '{') { strcat(statement,"}"); }

	} else { l_back(); }

	return statement;
}

char* parseBrackets(char* buffer)
{
	char* statement = malloc(1);
	char* line;
	strcat(statement,"{");
	l_scan(buffer);
	if (strcmp(lexeme,"}") != 0)
	{
		while (1)
		{
			line = expr(buffer);
			if (strcmp(lexeme,"}") == 0) { break; }
			statement = realloc(statement,strlen(statement)+strlen(line)+1);
			strcat(statement,line);
			strcat(statement,";");
		}
	}
	l_scan(buffer);

	statement = realloc(statement,strlen(statement)+1);
	strcat(statement,"}");

	return statement;
}

char* parseParentheses(char* buffer)
{
	char* statement = expr(buffer);
	l_scan(buffer);
	if (strcmp(lexeme,")") != 0) { fprintf(stderr, "Error: mismatched parentheses."); exit(1); }
	return statement;
}
