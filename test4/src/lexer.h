#ifndef SRC_LEXER_H
#define SRC_LEXER_H

typedef enum
{
	T_BINOP,
	T_PUNC,
	T_NUM,
	T_STR,
	T_BOOL,
	T_KEYWORD,
	T_VAR,
	T_END
} tokentype;

tokentype type;
char* lexeme;

void l_init(void);
void l_free(void);
void l_peek(char* buffer);
void l_scan(char* buffer);
void l_back(void);

#endif
