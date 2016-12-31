#include <ctype.h>
#include "lexer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int pos = 0;
static int lastpos = 0;

static char* binops[] = { "+","-","*","/","%","==","<=",">=","!=","<",">","|","&" };
static char* punc = "{}();,";
static char* keywords[] = { "func","if","else" };

void l_init(void) { lexeme = malloc(2); }

void l_free(void) { free(lexeme); }

void l_peek(char* buffer)
{	
	while (pos < strlen(buffer) && isspace(buffer[pos])) { pos++; }

	if (pos >= strlen(buffer)) { type = T_END; return; }

	for (int i=0;i<sizeof(binops)/sizeof(char*);i++)
	{
		if (strncmp(binops[i], buffer+pos, strlen(binops[i])) == 0)
		{
			type = T_BINOP;
			return;
		}
	}

	if (strchr(punc, buffer[pos]) != NULL) { type = T_PUNC; return; }
	if (isdigit(buffer[pos])) { type = T_NUM; return; }
	if (buffer[pos] == '\'' || buffer[pos] == '\"') { type = T_STR; return; }
	if (isalpha(buffer[pos]) || buffer[pos] == '_') { type = T_VAR; return; }
}

void l_scan(char* buffer)
{
	lastpos = pos;
	l_peek(buffer);
	if (pos >= strlen(buffer)) { type = T_END; return; }
	long num;
	char end;
	switch (type)
	{
		case T_BINOP:
			for (int i=0;i<sizeof(binops)/sizeof(char*);i++)
			{
				if (strncmp(binops[i], buffer+pos, strlen(binops[i])) == 0)
				{
					lexeme = realloc(lexeme,strlen(binops[i]) + 1);
					strcpy(lexeme,binops[i]);
					lexeme[strlen(binops[i])] = 0;
					pos += strlen(binops[i]);
					break;
				}
			}
			break;
		case T_PUNC:
			lexeme = realloc(lexeme, 2);
			lexeme[0] = buffer[pos++];
			lexeme[1] = 0;
			break;
		case T_NUM:
			num = buffer[pos++] - '0';
			while (isdigit(buffer[pos]))
			{
				num = num * 10 + (buffer[pos++] - '0');
			}
			if (num == 0)
			{
				lexeme = realloc(lexeme, 2);
			}
			else { lexeme = realloc(lexeme, floor(log10(abs(num))) + 2); }
			sprintf(lexeme, "%ld", num);
			lexeme[strlen(lexeme)] = 0;
			break;
		case T_STR:
			lexeme = realloc(lexeme,1);
			end = buffer[pos++];
			int i;
			for (i=0;buffer[pos] != end && pos < strlen(buffer);i++,pos++)
			{
				lexeme[i] = buffer[pos];
				lexeme = realloc(lexeme,i+2);
			}
			lexeme[i] = 0;
			pos++;
			break;
		case T_VAR:
			lexeme = realloc(lexeme,1);
			for (i=0;pos < strlen(buffer) && (isalnum(buffer[pos]) || buffer[pos] == '_');i++,pos++)
			{
				lexeme[i] = buffer[pos];
				lexeme = realloc(lexeme,i+2);
			}
			lexeme[i] = 0;
			if (strcmp(lexeme,"true") == 0 || strcmp(lexeme,"false") == 0) { type = T_BOOL; }
			for (int i=0;i<sizeof(keywords)/sizeof(char*);i++)
			{
				if (strcmp(keywords[i],lexeme) == 0) { type = T_KEYWORD; break; }
			}
			break;
		default:
			break;
	}
}

void l_back(void)
{
	pos = lastpos;
}
